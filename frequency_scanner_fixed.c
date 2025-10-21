#include "fpv_interceptor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

// Глобальные переменные
static int running = 1;
static int scan_running = 0;
static int signal_count = 0;
#define MAX_SIGNALS 100
#define SCAN_DWELL_TIME 100

// Структура канала
typedef struct {
    uint8_t rssi_index;
    uint8_t rssi_smoothed;
} channel_info_t;

static channel_info_t channels[CHANNELS_COUNT];

// Объявления функций
static int scan_single_frequency(uint16_t frequency);
static void print_status(uint16_t freq, uint8_t rssi);

/**
 * Инициализация частотного сканера
 * @return 0 при успехе, -1 при ошибке
 */
int frequency_scanner_init(void) {
    printf("🔍 Инициализация частотного сканера...\n");
    
    // Инициализация каналов
    for (int i = 0; i < CHANNELS_COUNT; i++) {
        channels[i].rssi_index = 0;
        channels[i].rssi_smoothed = 0;
    }
    
    // Инициализация счетчиков
    signal_count = 0;
    for (int i = 0; i < MAX_SIGNALS; i++) {
        detected_signals[i].frequency = 0;
        detected_signals[i].rssi = 0;
        detected_signals[i].timestamp = 0;
        detected_signals[i].motion_detected = 0;
        detected_signals[i].video_quality = 0;
    }
    
    printf("✅ Частотный сканер инициализирован\n");
    return 0;
}

/**
 * Сканирование диапазона частот
 * @param start_freq Начальная частота
 * @param end_freq Конечная частота
 * @param dwell_time Время задержки на частоте (мс)
 * @return 0 при успехе, -1 при ошибке
 */
int scan_frequency_range(uint16_t start_freq, uint16_t end_freq, int dwell_time) {
    if (start_freq < FREQ_MIN || end_freq > FREQ_MAX) {
        printf("❌ Неверный диапазон частот\n");
        return -1;
    }
    
    printf("🔍 Сканирование диапазона %d-%d МГц...\n", start_freq, end_freq);
    
    for (uint16_t freq = start_freq; freq <= end_freq && running; freq += FREQ_STEP) {
        if (scan_single_frequency(freq) == 0) {
            uint8_t rssi = analyze_rssi(freq);
            int channel = freq - FREQ_MIN;
            
            if (channel >= 0 && channel < CHANNELS_COUNT) {
                channels[channel].rssi_smoothed = rssi;
            }
            
            if (rssi > RSSI_THRESHOLD) {
                printf("🎯 Сигнал обнаружен: %d МГц, RSSI: %d%%\n", freq, rssi);
                add_detected_signal(freq, rssi, "FPV");
            }
            
            print_status(freq, rssi);
        }
        
        usleep(dwell_time * 1000);
    }
    
    return 0;
}

/**
 * Сканирование одной частоты
 * @param frequency Частота для сканирования
 * @return 0 при успехе, -1 при ошибке
 */
static int scan_single_frequency(uint16_t frequency) {
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) {
        printf("❌ Частота %d МГц вне диапазона\n", frequency);
        return -1;
    }
    
    // Установка частоты на RX5808
    if (rx5808_set_frequency(frequency) != 0) {
        printf("❌ Ошибка установки частоты %d МГц\n", frequency);
        return -1;
    }
    
    // Ожидание стабилизации
    usleep(50000); // 50 мс
    
    return 0;
}

/**
 * Непрерывное сканирование
 * @return 0 при успехе, -1 при ошибке
 */
int scan_continuous(void) {
    printf("🔄 Непрерывное сканирование...\n");
    scan_running = 1;
    
    while (running && scan_running) {
        for (uint16_t freq = FREQ_MIN; freq <= FREQ_MAX && running; freq += FREQ_STEP) {
            if (scan_single_frequency(freq) == 0) {
                uint8_t rssi = analyze_rssi(freq);
                int channel = freq - FREQ_MIN;
                
                if (channel >= 0 && channel < CHANNELS_COUNT) {
                    channels[channel].rssi_smoothed = rssi;
                }
                
                if (rssi > RSSI_THRESHOLD) {
                    printf("🎯 Сигнал обнаружен: %d МГц, RSSI: %d%%\n", freq, rssi);
                    add_detected_signal(freq, rssi, "FPV");
                }
                
                print_status(freq, rssi);
            }
            
            usleep(SCAN_DWELL_TIME * 1000);
        }
    }
    
    scan_running = 0;
    return 0;
}

/**
 * Мониторинг конкретной частоты
 * @param frequency Частота для мониторинга
 * @param timeout_ms Таймаут в миллисекундах
 * @return 0 при успехе, -1 при ошибке
 */
int monitor_frequency(uint16_t frequency, int timeout_ms) {
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) {
        printf("❌ Частота %d МГц вне диапазона\n", frequency);
        return -1;
    }
    
    printf("👁️ Мониторинг частоты %d МГц...\n", frequency);
    
    uint32_t start_time = get_timestamp();
    uint32_t timeout = timeout_ms;
    
    while (running) {
        if (scan_single_frequency(frequency) == 0) {
            uint8_t rssi = analyze_rssi(frequency);
            int channel = frequency - FREQ_MIN;
            
            if (channel >= 0 && channel < CHANNELS_COUNT) {
                channels[channel].rssi_smoothed = rssi;
            }
            
            if (rssi > RSSI_THRESHOLD) {
                printf("🎯 Сигнал обнаружен: %d МГц, RSSI: %d%%\n", frequency, rssi);
                add_detected_signal(frequency, rssi, "FPV");
                return 0;
            }
            
            print_status(frequency, rssi);
        }
        
        // Проверка таймаута
        if (timeout > 0 && (get_timestamp() - start_time) > timeout) {
            printf("⏰ Таймаут мониторинга\n");
            break;
        }
        
        usleep(100000); // 100 мс
    }
    
    return -1;
}

/**
 * Автоматическое сканирование для поиска сигналов
 * @return Количество найденных сигналов
 */
int auto_scan_for_signals(void) {
    printf("🤖 Автоматический поиск сигналов...\n");
    
    int found_signals = 0;
    signal_count = 0;
    
    for (uint16_t freq = FREQ_MIN; freq <= FREQ_MAX && running; freq += FREQ_STEP) {
        if (scan_single_frequency(freq) == 0) {
            uint8_t rssi = analyze_rssi(freq);
            
            if (rssi > RSSI_THRESHOLD) {
                printf("🎯 Сигнал найден: %d МГц, RSSI: %d%%\n", freq, rssi);
                add_detected_signal(freq, rssi, "FPV");
                found_signals++;
            }
        }
        
        usleep(SCAN_DWELL_TIME * 1000);
    }
    
    printf("✅ Найдено сигналов: %d\n", found_signals);
    return found_signals;
}

/**
 * Печать статуса сканирования
 * @param freq Текущая частота
 * @param rssi Текущий RSSI
 */
static void print_status(uint16_t freq, uint8_t rssi) {
    printf("📡 %d МГц: RSSI %d%%\r", freq, rssi);
    fflush(stdout);
}

/**
 * Получение статистики сканирования
 */
void get_scan_stats(void) {
    printf("📊 Статистика сканирования:\n");
    printf("   Обнаружено сигналов: %d\n", signal_count);
    printf("   Диапазон: %d-%d МГц\n", FREQ_MIN, FREQ_MAX);
    printf("   Шаг: %d МГц\n", FREQ_STEP);
    printf("   Статус: %s\n", scan_running ? "Активно" : "Остановлено");
}

/**
 * Очистка ресурсов сканера
 */
void frequency_scanner_cleanup(void) {
    printf("🧹 Очистка частотного сканера...\n");
    
    scan_running = 0;
    running = 0;
    
    // Очистка каналов
    for (int i = 0; i < CHANNELS_COUNT; i++) {
        channels[i].rssi_index = 0;
        channels[i].rssi_smoothed = 0;
    }
    
    printf("✅ Частотный сканер очищен\n");
}
