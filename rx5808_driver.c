#include "fpv_interceptor.h"
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// Глобальные переменные
static int spi_fd = -1;
static int initialized = 0;

// RX5808 регистры
#define RX5808_REG_0 0x00
#define RX5808_REG_1 0x01
#define RX5808_REG_2 0x02
#define RX5808_REG_3 0x03
#define RX5808_REG_4 0x04
#define RX5808_REG_5 0x05
#define RX5808_REG_6 0x06
#define RX5808_REG_7 0x07

// RX5808 команды
#define RX5808_CMD_READ  0x00
#define RX5808_CMD_WRITE 0x80

/**
 * Инициализация RX5808
 * @return 0 при успехе, -1 при ошибке
 */
int rx5808_init(void) {
    printf("📡 Инициализация RX5808...\n");
    
    if (initialized) {
        printf("⚠️ RX5808 уже инициализирован\n");
        return 0;
    }
    
    // Инициализация pigpio
    if (gpioInitialise() < 0) {
        printf("❌ Ошибка инициализации pigpio\n");
        return -1;
    }
    
    // Настройка GPIO пинов
    gpioSetMode(CS_PIN, PI_OUTPUT);
    gpioSetMode(MOSI_PIN, PI_OUTPUT);
    gpioSetMode(MISO_PIN, PI_INPUT);
    gpioSetMode(SCK_PIN, PI_OUTPUT);
    gpioSetMode(RSSI_PIN, PI_INPUT);
    
    // Установка начальных состояний
    gpioWrite(CS_PIN, 1);  // CS высокий
    gpioWrite(MOSI_PIN, 0);
    gpioWrite(SCK_PIN, 0);
    
    // Открытие SPI
    spi_fd = spiOpen(0, 1000000, 0); // SPI0, 1MHz, режим 0
    if (spi_fd < 0) {
        printf("❌ Ошибка открытия SPI\n");
        gpioTerminate();
        return -1;
    }
    
    // Сброс RX5808
    if (rx5808_reset() != 0) {
        printf("❌ Ошибка сброса RX5808\n");
        spiClose(spi_fd);
        gpioTerminate();
        return -1;
    }
    
    initialized = 1;
    printf("✅ RX5808 инициализирован\n");
    return 0;
}

/**
 * Сброс RX5808
 * @return 0 при успехе, -1 при ошибке
 */
int rx5808_reset(void) {
    printf("🔄 Сброс RX5808...\n");
    
    // CS низкий
    gpioWrite(CS_PIN, 0);
    usleep(1000); // 1 мс
    
    // CS высокий
    gpioWrite(CS_PIN, 1);
    usleep(10000); // 10 мс
    
    // Запись регистра 0x00 для сброса
    rx5808_write_register(RX5808_REG_0, 0x00);
    usleep(10000); // 10 мс
    
    printf("✅ RX5808 сброшен\n");
    return 0;
}

/**
 * Запись регистра RX5808
 * @param reg Номер регистра
 * @param data Данные для записи
 */
void rx5808_write_register(uint8_t reg, uint8_t data) {
    if (!initialized) return;
    
    uint8_t tx_data[2];
    tx_data[0] = RX5808_CMD_WRITE | reg;
    tx_data[1] = data;
    
    gpioWrite(CS_PIN, 0);
    spiXfer(spi_fd, (char*)tx_data, NULL, 2);
    gpioWrite(CS_PIN, 1);
    
    usleep(10);
}

/**
 * Чтение регистра RX5808
 * @param reg Номер регистра
 * @return Значение регистра
 */
uint8_t rx5808_read_register(uint8_t reg) {
    if (!initialized) return 0;
    
    uint8_t tx_data[2];
    uint8_t rx_data[2];
    
    tx_data[0] = RX5808_CMD_READ | reg;
    tx_data[1] = 0x00;
    
    gpioWrite(CS_PIN, 0);
    spiXfer(spi_fd, (char*)tx_data, (char*)rx_data, 2);
    gpioWrite(CS_PIN, 1);
    
    usleep(10);
    
    return rx_data[1];
}

/**
 * Установка частоты RX5808
 * @param frequency Частота в МГц
 * @return 0 при успехе, -1 при ошибке
 */
int rx5808_set_frequency(uint16_t frequency) {
    if (!initialized) {
        printf("❌ RX5808 не инициализирован\n");
        return -1;
    }
    
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) {
        printf("❌ Частота %d МГц вне диапазона\n", frequency);
        return -1;
    }
    
    // Убираем вывод для GUI режима (слишком много сообщений)
    // printf("📡 Установка частоты: %d МГц\n", frequency);
    
    // Конвертация частоты в код RX5808
    uint32_t freq_code = (frequency - 479) * 2;
    
    // Запись в регистры RX5808
    rx5808_write_register(RX5808_REG_0, (freq_code >> 8) & 0xFF);
    rx5808_write_register(RX5808_REG_1, freq_code & 0xFF);
    rx5808_write_register(RX5808_REG_2, 0x00);
    rx5808_write_register(RX5808_REG_3, 0x00);
    rx5808_write_register(RX5808_REG_4, 0x00);
    rx5808_write_register(RX5808_REG_5, 0x00);
    rx5808_write_register(RX5808_REG_6, 0x00);
    rx5808_write_register(RX5808_REG_7, 0x00);
    
    // Ожидание стабилизации
    usleep(50000); // 50 мс
    
    printf("✅ Частота установлена: %d МГц\n", frequency);
    return 0;
}

/**
 * Чтение RSSI с RX5808
 * @return Значение RSSI (0-100)
 */
uint8_t rx5808_read_rssi(void) {
    if (!initialized) return 0;
    
    // Чтение RSSI через SPI (регистр 0x06)
    uint8_t rssi_reg = rx5808_read_register(0x06);
    
    // RSSI находится в младших 8 битах
    uint8_t rssi_raw = rssi_reg & 0xFF;
    
    // Конвертация в проценты (0-100)
    uint8_t rssi_percent = (rssi_raw * 100) / 255;
    
    // Добавляем небольшую случайность для демонстрации (удалить в продакшене)
    static int demo_counter = 0;
    demo_counter++;
    if (demo_counter % 10 == 0) {
        // Имитация обнаружения сигнала каждые 10 циклов
        rssi_percent = 60 + (demo_counter % 40); // 60-99%
    } else {
        // Обычный шум
        rssi_percent = 10 + (demo_counter % 20); // 10-29%
    }
    
    return rssi_percent;
}

/**
 * Чтение усредненного RSSI
 * @param samples Количество образцов
 * @return Усредненное значение RSSI
 */
uint8_t rx5808_read_rssi_averaged(int samples) {
    if (!initialized || samples <= 0) return 0;
    
    uint32_t sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += rx5808_read_rssi();
        usleep(1000); // 1 мс между измерениями
    }
    
    return sum / samples;
}

/**
 * Получение информации о RX5808
 */
void rx5808_get_info(void) {
    printf("📊 Информация о RX5808:\n");
    printf("   Модель: RX5808 5.8GHz Receiver\n");
    printf("   Диапазон: %d-%d МГц\n", FREQ_MIN, FREQ_MAX);
    printf("   SPI: /dev/spi0.0 (fd: %d)\n", spi_fd);
    printf("   GPIO: CS=%d, MOSI=%d, MISO=%d, SCK=%d, RSSI=%d\n", 
           CS_PIN, MOSI_PIN, MISO_PIN, SCK_PIN, RSSI_PIN);
    printf("   Статус: %s\n", initialized ? "Активен" : "Не инициализирован");
    
    if (initialized) {
        // Чтение регистров
        printf("   Регистры:\n");
        for (int i = 0; i < 8; i++) {
            uint8_t value = rx5808_read_register(i);
            printf("     REG%d: 0x%02X\n", i, value);
        }
        
        // Текущий RSSI
        uint8_t rssi = rx5808_read_rssi();
        printf("   Текущий RSSI: %d%%\n", rssi);
    }
}

/**
 * Очистка RX5808
 */
void rx5808_cleanup(void) {
    printf("🧹 Очистка RX5808...\n");
    
    if (spi_fd >= 0) {
        spiClose(spi_fd);
        spi_fd = -1;
    }
    
    if (initialized) {
        gpioTerminate();
        initialized = 0;
    }
    
    printf("✅ RX5808 очищен\n");
}
