#include "fpv_interceptor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// Глобальные переменные
detected_signal_t detected_signals[100];
int detected_count = 0;

/**
 * Получение временной метки
 * @return Временная метка в миллисекундах
 */
uint32_t get_timestamp(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/**
 * Добавление обнаруженного сигнала
 * @param frequency Частота сигнала
 * @param rssi Уровень RSSI
 * @param signal_type Тип сигнала
 */
void add_detected_signal(uint16_t frequency, uint8_t rssi, const char* signal_type) {
    if (detected_count >= 100) {
        printf("⚠️ Превышено максимальное количество сигналов\n");
        return;
    }
    
    detected_signals[detected_count].frequency = frequency;
    detected_signals[detected_count].rssi = rssi;
    detected_signals[detected_count].timestamp = get_timestamp();
    detected_signals[detected_count].motion_detected = 0;
    detected_signals[detected_count].video_quality = 0;
    
    if (signal_type) {
        strncpy(detected_signals[detected_count].filename, 
                signal_type, sizeof(detected_signals[detected_count].filename) - 1);
        detected_signals[detected_count].filename[sizeof(detected_signals[detected_count].filename) - 1] = '\0';
    }
    
    detected_count++;
    
    printf("📝 Сигнал добавлен: %d МГц, RSSI: %d%%, Тип: %s\n", 
           frequency, rssi, signal_type ? signal_type : "Неизвестно");
}

/**
 * Печать обнаруженных сигналов
 */
void print_detected_signals(void) {
    printf("\n📊 Обнаруженные сигналы:\n");
    printf("┌─────────────┬─────────┬─────────┬─────────────────────┬─────────────────┐\n");
    printf("│ Частота МГц │ RSSI %  │ Тип     │ Время обнаружения   │ Дополнительно   │\n");
    printf("├─────────────┼─────────┼─────────┼─────────────────────┼─────────────────┤\n");
    
    for (int i = 0; i < detected_count; i++) {
        printf("│ %-11d │ %-7d │ %-7s │ %-19u │ %-15s │\n",
               detected_signals[i].frequency,
               detected_signals[i].rssi,
               detected_signals[i].filename,
               detected_signals[i].timestamp,
               detected_signals[i].motion_detected ? "Движение" : "Нет движения");
    }
    
    printf("└─────────────┴─────────┴─────────┴─────────────────────┴─────────────────┘\n");
    printf("Всего сигналов: %d\n", detected_count);
}

/**
 * Сохранение данных сигналов в файл
 */
void save_signal_data(void) {
    char filename[256];
    snprintf(filename, sizeof(filename), "signals_%u.txt", get_timestamp());
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("❌ Ошибка создания файла: %s\n", filename);
        return;
    }
    
    fprintf(file, "# FPV Interceptor - Данные сигналов\n");
    fprintf(file, "# Время сканирования: %u\n", get_timestamp());
    fprintf(file, "# Формат: Частота, RSSI, Время, Тип, Движение, Качество\n");
    fprintf(file, "\n");
    
    for (int i = 0; i < detected_count; i++) {
        fprintf(file, "%d,%d,%u,%s,%d,%d\n",
                detected_signals[i].frequency,
                detected_signals[i].rssi,
                detected_signals[i].timestamp,
                detected_signals[i].filename,
                detected_signals[i].motion_detected,
                detected_signals[i].video_quality);
    }
    
    fclose(file);
    printf("💾 Данные сохранены: %s\n", filename);
}

/**
 * Очистка ресурсов
 */
void cleanup_resources(void) {
    printf("🧹 Очистка ресурсов...\n");
    
    // Очистка обнаруженных сигналов
    detected_count = 0;
    memset(detected_signals, 0, sizeof(detected_signals));
    
    printf("✅ Ресурсы очищены\n");
}
