#include "fpv_interceptor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Инициализация RX5808 (заглушка)
 */
int rx5808_init(void) {
    printf("📡 RX5808 инициализирован (заглушка)\n");
    return 0;
}

/**
 * Сброс RX5808 (заглушка)
 */
int rx5808_reset(void) {
    printf("🔄 RX5808 сброс (заглушка)\n");
    return 0;
}

/**
 * Запись регистра RX5808 (заглушка)
 */
void rx5808_write_register(uint8_t reg, uint8_t data) {
    printf("📝 Запись регистра RX5808: 0x%02X = 0x%02X (заглушка)\n", reg, data);
}

/**
 * Чтение регистра RX5808 (заглушка)
 */
uint8_t rx5808_read_register(uint8_t reg) {
    printf("📖 Чтение регистра RX5808: 0x%02X (заглушка)\n", reg);
    return 0x00;
}

/**
 * Установка частоты RX5808 (заглушка)
 */
int rx5808_set_frequency(uint16_t frequency) {
    printf("📡 Установка частоты RX5808: %d МГц (заглушка)\n", frequency);
    return 0;
}

/**
 * Чтение RSSI RX5808 (заглушка)
 */
uint8_t rx5808_read_rssi(void) {
    // Возвращаем случайный RSSI для демонстрации
    return 30 + (rand() % 40); // 30-70%
}

/**
 * Чтение усредненного RSSI (заглушка)
 */
uint8_t rx5808_read_rssi_averaged(int samples) {
    printf("📊 Чтение усредненного RSSI: %d образцов (заглушка)\n", samples);
    return rx5808_read_rssi();
}

/**
 * Получение информации о RX5808 (заглушка)
 */
void rx5808_get_info(void) {
    printf("📊 Информация о RX5808 (заглушка):\n");
    printf("   Модель: RX5808 5.8GHz Receiver\n");
    printf("   Диапазон: 5725-6000 МГц\n");
    printf("   Статус: Заглушка (без реального оборудования)\n");
}

/**
 * Очистка RX5808 (заглушка)
 */
void rx5808_cleanup(void) {
    printf("🧹 Очистка RX5808 (заглушка)\n");
}
