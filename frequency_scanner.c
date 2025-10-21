#include "fpv_interceptor.h"
#include <pthread.h>

// Глобальные переменные для сканирования
static pthread_t scan_thread;
static int scan_running = 0;
static uint16_t current_frequency = FREQ_MIN;
static uint8_t scan_mode = 0; // 0 = полное сканирование, 1 = непрерывное

/**
 * Инициализация сканера частот
 * @return 0 при успехе, -1 при ошибке
 */
int frequency_scanner_init(void) {
    printf("📡 Инициализация сканера частот...\n");
    
    // Инициализация данных каналов
    for (int i = 0; i < CHANNELS_COUNT; i++) {
        channels[i].rssi_index = 0;
        channels[i].rssi_smoothed = 0;
        channels[i].video_detected = 0;
        channels[i].last_update = 0;
        
        for (int j = 0; j < RSSI_SAMPLES; j++) {
            channels[i].rssi_history[j] = 0;
        }
    }
    
    // Инициализация обнаруженных сигналов
    signal_count = 0;
    for (int i = 0; i < MAX_SIGNALS; i++) {
        detected_signals[i].frequency = 0;
        detected_signals[i].rssi = 0;
        detected_signals[i].video_detected = 0;
        detected_signals[i].timestamp = 0;
        memset(detected_signals[i].video_file, 0, sizeof(detected_signals[i].video_file));
    }
    
    printf("✅ Сканер частот инициализирован\n");
    return 0;
}

/**
 * Полное сканирование диапазона частот
 * @return 0 при успехе, -1 при ошибке
 */
int scan_frequency_range(void) {
    printf("🔍 Начало полного сканирования диапазона %d-%d МГц...\n", 
           FREQ_MIN, FREQ_MAX);
    
    scan_mode = 0;
    scan_running = 1;
    
    for (uint16_t freq = FREQ_MIN; freq <= FREQ_MAX && running; freq += FREQ_STEP) {
        if (scan_single_frequency(freq) == 0) {
            printf("📡 Сканирование %d МГц...\n", freq);
            
            // Анализ RSSI
            uint8_t rssi = analyze_rssi(freq);
            
            // Обнаружение видеосигнала
            uint8_t video_detected = detect_video_signal(freq);
            
            // Сохранение данных канала
            int channel = freq - FREQ_MIN;
            channels[channel].rssi_smoothed = rssi;
            channels[channel].video_detected = video_detected;
            channels[channel].last_update = get_timestamp();
            
            // Если обнаружен сигнал, добавить в список
            if (rssi > RSSI_THRESHOLD) {
                add_detected_signal(freq, rssi, video_detected);
            }
            
            // Задержка для стабилизации
            usleep(SCAN_DWELL_TIME * 1000);
        }
    }
    
    scan_running = 0;
    printf("✅ Полное сканирование завершено\n");
    
    // Вывод результатов
    print_detected_signals();
    
    return 0;
}

/**
 * Сканирование одной частоты
 * @param frequency Частота для сканирования
 * @return 0 при успехе, -1 при ошибке
 */
int scan_single_frequency(uint16_t frequency) {
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) {
        printf("❌ Частота %d МГц вне диапазона\n", frequency);
        return -1;
    }
    
    // Установка частоты на RX5808 (используем Linux GPIO версию)
    if (rx5808_set_frequency_linux(frequency) != 0) {
        printf("❌ Ошибка установки частоты %d МГц\n", frequency);
        return -1;
    }
    
    // Ожидание стабилизации
    usleep(50000); // 50 мс
    
    return 0;
}

/**
 * Непрерывное сканирование (в отдельном потоке)
 */
void scan_continuous(void) {
    printf("🔄 Начало непрерывного сканирования...\n");
    
    scan_mode = 1;
    scan_running = 1;
    
    while (running && scan_running) {
        for (uint16_t freq = FREQ_MIN; freq <= FREQ_MAX && running; freq += FREQ_STEP) {
            if (!running) break;
            
            // Сканирование частоты
            if (scan_single_frequency(freq) == 0) {
                current_frequency = freq;
                
                // Анализ сигнала
                uint8_t rssi = analyze_rssi(freq);
                uint8_t video_detected = detect_video_signal(freq);
                
                // Обновление данных канала
                int channel = freq - FREQ_MIN;
                channels[channel].rssi_smoothed = rssi;
                channels[channel].video_detected = video_detected;
                channels[channel].last_update = get_timestamp();
                
                // Проверка на обнаружение сигнала
                if (rssi > RSSI_THRESHOLD) {
                    printf("🎯 Сигнал обнаружен: %d МГц, RSSI: %d%%, Видео: %s\n", 
                           freq, rssi, video_detected ? "Да" : "Нет");
                    
                    // Добавление в список обнаруженных
                    add_detected_signal(freq, rssi, video_detected);
                    
                    // Если обнаружено видео, начать захват
                    if (video_detected) {
                        printf("📹 Захват видеосигнала на %d МГц...\n", freq);
                        capture_video_frame(freq);
                        save_video_stream(freq);
                    }
                }
                
                // Вывод статуса
                print_status(freq, rssi);
            }
            
            // Задержка между частотами
            usleep(SCAN_DWELL_TIME * 1000);
        }
        
        // Пауза между циклами сканирования
        if (running) {
            printf("🔄 Цикл сканирования завершен, пауза 5 сек...\n");
            sleep(5);
        }
    }
    
    scan_running = 0;
    printf("✅ Непрерывное сканирование остановлено\n");
}

/**
 * Функция потока для непрерывного сканирования
 * @param arg Аргументы потока
 * @return NULL
 */
void* scan_thread_function(void* arg) {
    (void)arg;
    scan_continuous();
    return NULL;
}

/**
 * Запуск непрерывного сканирования в отдельном потоке
 * @return 0 при успехе, -1 при ошибке
 */
int start_continuous_scan(void) {
    if (scan_running) {
        printf("⚠️ Сканирование уже запущено\n");
        return -1;
    }
    
    printf("🚀 Запуск непрерывного сканирования в отдельном потоке...\n");
    
    if (pthread_create(&scan_thread, NULL, scan_thread_function, NULL) != 0) {
        printf("❌ Ошибка создания потока сканирования\n");
        return -1;
    }
    
    printf("✅ Поток сканирования запущен\n");
    return 0;
}

/**
 * Остановка непрерывного сканирования
 * @return 0 при успехе, -1 при ошибке
 */
int stop_continuous_scan(void) {
    if (!scan_running) {
        printf("⚠️ Сканирование не запущено\n");
        return -1;
    }
    
    printf("⏹️ Остановка непрерывного сканирования...\n");
    
    scan_running = 0;
    
    // Ожидание завершения потока
    if (pthread_join(scan_thread, NULL) != 0) {
        printf("❌ Ошибка ожидания завершения потока\n");
        return -1;
    }
    
    printf("✅ Непрерывное сканирование остановлено\n");
    return 0;
}

/**
 * Сканирование конкретного диапазона частот
 * @param start_freq Начальная частота
 * @param end_freq Конечная частота
 * @return 0 при успехе, -1 при ошибке
 */
int scan_frequency_range_custom(uint16_t start_freq, uint16_t end_freq) {
    if (start_freq < FREQ_MIN || end_freq > FREQ_MAX || start_freq > end_freq) {
        printf("❌ Неверный диапазон частот: %d-%d МГц\n", start_freq, end_freq);
        return -1;
    }
    
    printf("🔍 Сканирование диапазона %d-%d МГц...\n", start_freq, end_freq);
    
    for (uint16_t freq = start_freq; freq <= end_freq && running; freq += FREQ_STEP) {
        if (scan_single_frequency(freq) == 0) {
            printf("📡 Сканирование %d МГц...\n", freq);
            
            // Анализ сигнала
            uint8_t rssi = analyze_rssi(freq);
            uint8_t video_detected = detect_video_signal(freq);
            
            // Сохранение данных
            int channel = freq - FREQ_MIN;
            channels[channel].rssi_smoothed = rssi;
            channels[channel].video_detected = video_detected;
            channels[channel].last_update = get_timestamp();
            
            // Проверка на обнаружение
            if (rssi > RSSI_THRESHOLD) {
                printf("🎯 Сигнал: %d МГц, RSSI: %d%%, Видео: %s\n", 
                       freq, rssi, video_detected ? "Да" : "Нет");
                
                add_detected_signal(freq, rssi, video_detected);
                
                if (video_detected) {
                    capture_video_frame(freq);
                }
            }
            
            usleep(SCAN_DWELL_TIME * 1000);
        }
    }
    
    printf("✅ Сканирование диапазона завершено\n");
    return 0;
}

/**
 * Мониторинг конкретной частоты
 * @param frequency Частота для мониторинга
 * @param duration Время мониторинга в секундах (0 = бесконечно)
 * @return 0 при успехе, -1 при ошибке
 */
int monitor_frequency(uint16_t frequency, uint32_t duration) {
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) {
        printf("❌ Частота %d МГц вне диапазона\n", frequency);
        return -1;
    }
    
    printf("👁️ Мониторинг частоты %d МГц", frequency);
    if (duration > 0) {
        printf(" в течение %d секунд", duration);
    } else {
        printf(" (бесконечно)");
    }
    printf("...\n");
    
    // Установка частоты (используем Linux GPIO версию)
    if (rx5808_set_frequency_linux(frequency) != 0) {
        printf("❌ Ошибка установки частоты\n");
        return -1;
    }
    
    uint32_t start_time = get_timestamp();
    uint32_t last_rssi_update = 0;
    uint8_t last_rssi = 0;
    
    while (running) {
        // Проверка времени
        if (duration > 0 && (get_timestamp() - start_time) > duration * 1000) {
            break;
        }
        
        // Анализ сигнала
        uint8_t rssi = analyze_rssi(frequency);
        uint8_t video_detected = detect_video_signal(frequency);
        
        // Обновление данных канала
        int channel = frequency - FREQ_MIN;
        channels[channel].rssi_smoothed = rssi;
        channels[channel].video_detected = video_detected;
        channels[channel].last_update = get_timestamp();
        
        // Вывод изменений RSSI
        if (abs(rssi - last_rssi) > 5 || (get_timestamp() - last_rssi_update) > 1000) {
            printf("📊 %d МГц: RSSI=%d%%, Видео=%s\n", 
                   frequency, rssi, video_detected ? "Да" : "Нет");
            last_rssi = rssi;
            last_rssi_update = get_timestamp();
        }
        
        // Если обнаружен видеосигнал
        if (video_detected && rssi > RSSI_THRESHOLD) {
            printf("📹 Видеосигнал обнаружен на %d МГц!\n", frequency);
            capture_video_frame(frequency);
            add_detected_signal(frequency, rssi, 1);
        }
        
        usleep(100000); // 100 мс
    }
    
    printf("✅ Мониторинг частоты %d МГц завершен\n", frequency);
    return 0;
}

/**
 * Получение статистики сканирования
 */
void get_scan_stats(void) {
    printf("📊 Статистика сканирования:\n");
    printf("   Режим: %s\n", scan_mode ? "Непрерывный" : "Полный");
    printf("   Статус: %s\n", scan_running ? "Активен" : "Остановлен");
    printf("   Текущая частота: %d МГц\n", current_frequency);
    printf("   Обнаружено сигналов: %d\n", signal_count);
    printf("   Диапазон: %d-%d МГц\n", FREQ_MIN, FREQ_MAX);
    printf("   Шаг: %d МГц\n", FREQ_STEP);
}

/**
 * Очистка ресурсов сканера
 */
void frequency_scanner_cleanup(void) {
    printf("🧹 Очистка ресурсов сканера...\n");
    
    if (scan_running) {
        stop_continuous_scan();
    }
    
    // Очистка данных каналов
    for (int i = 0; i < CHANNELS_COUNT; i++) {
        channels[i].rssi_index = 0;
        channels[i].rssi_smoothed = 0;
        channels[i].video_detected = 0;
        channels[i].last_update = 0;
        
        for (int j = 0; j < RSSI_SAMPLES; j++) {
            channels[i].rssi_history[j] = 0;
        }
    }
    
    printf("✅ Ресурсы сканера очищены\n");
}

