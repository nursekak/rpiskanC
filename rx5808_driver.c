#include "fpv_interceptor.h"
#include <pigpio.h>
#include <unistd.h>

// Глобальные переменные для SPI
static int spi_fd = -1;
static int spi_initialized = 0;

/**
 * Инициализация модуля RX5808
 * @return 0 при успехе, -1 при ошибке
 */
int rx5808_init(void) {
    printf("🔧 Инициализация модуля RX5808...\n");
    
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
    gpioWrite(CS_PIN, 1);
    gpioWrite(MOSI_PIN, 0);
    gpioWrite(SCK_PIN, 0);
    
    // Инициализация SPI
    spi_fd = spiOpen(0, 2000000, 0); // 2 MHz
    if (spi_fd < 0) {
        printf("❌ Ошибка инициализации SPI\n");
        return -1;
    }
    
    spi_initialized = 1;
    printf("✅ Модуль RX5808 инициализирован успешно\n");
    
    // Сброс модуля
    rx5808_reset();
    
    return 0;
}

/**
 * Сброс модуля RX5808
 */
void rx5808_reset(void) {
    if (!spi_initialized) return;
    
    printf("🔄 Сброс модуля RX5808...\n");
    
    // Последовательность сброса
    gpioWrite(CS_PIN, 0);
    usleep(100);
    gpioWrite(CS_PIN, 1);
    usleep(100);
    
    // Инициализация регистров
    rx5808_write_register(RX5808_REG_0A, 0x00);
    rx5808_write_register(RX5808_REG_0B, 0x00);
    rx5808_write_register(RX5808_REG_0C, 0x00);
    rx5808_write_register(RX5808_REG_0D, 0x00);
    rx5808_write_register(RX5808_REG_0E, 0x00);
    rx5808_write_register(RX5808_REG_0F, 0x00);
    
    usleep(10000);
    printf("✅ Сброс завершен\n");
}

/**
 * Запись в регистр RX5808
 * @param reg Адрес регистра
 * @param data Данные для записи
 */
void rx5808_write_register(uint8_t reg, uint8_t data) {
    if (!spi_initialized) return;
    
    uint8_t tx_data[2];
    tx_data[0] = RX5808_CMD_WRITE | reg;
    tx_data[1] = data;
    
    gpioWrite(CS_PIN, 0);
    spiXfer(spi_fd, tx_data, NULL, 2);
    gpioWrite(CS_PIN, 1);
    
    usleep(10);
}

/**
 * Чтение регистра RX5808
 * @param reg Адрес регистра
 * @return Значение регистра
 */
uint8_t rx5808_read_register(uint8_t reg) {
    if (!spi_initialized) return 0;
    
    uint8_t tx_data[2];
    uint8_t rx_data[2];
    
    tx_data[0] = RX5808_CMD_READ | reg;
    tx_data[1] = 0x00;
    
    gpioWrite(CS_PIN, 0);
    spiXfer(spi_fd, tx_data, rx_data, 2);
    gpioWrite(CS_PIN, 1);
    
    usleep(10);
    
    return rx_data[1];
}

/**
 * Установка частоты на модуле RX5808
 * @param frequency Частота в МГц
 * @return 0 при успехе, -1 при ошибке
 */
int rx5808_set_frequency(uint16_t frequency) {
    if (!spi_initialized) {
        printf("❌ SPI не инициализирован\n");
        return -1;
    }
    
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) {
        printf("❌ Частота %d МГц вне диапазона %d-%d МГц\n", 
               frequency, FREQ_MIN, FREQ_MAX);
        return -1;
    }
    
    printf("📡 Установка частоты: %d МГц\n", frequency);
    
    // Расчет параметров для RX5808
    uint32_t freq_reg = (frequency - 479) * 8192 / 1000;
    
    // Разбивка на байты
    uint8_t reg_0A = (freq_reg >> 8) & 0xFF;
    uint8_t reg_0B = freq_reg & 0xFF;
    
    // Запись в регистры
    rx5808_write_register(RX5808_REG_0A, reg_0A);
    rx5808_write_register(RX5808_REG_0B, reg_0B);
    
    // Включение синтезатора
    rx5808_write_register(RX5808_REG_0C, 0x01);
    
    // Ожидание стабилизации
    usleep(50000);
    
    printf("✅ Частота установлена: %d МГц\n", frequency);
    return 0;
}

/**
 * Чтение уровня RSSI
 * @return Уровень RSSI (0-100)
 */
uint8_t rx5808_read_rssi(void) {
    if (!spi_initialized) return 0;
    
    // Чтение аналогового значения RSSI
    uint8_t rssi_raw = gpioRead(RSSI_PIN);
    
    // Для более точного измерения можно использовать ADC
    // Здесь используется простое цифровое чтение
    uint8_t rssi_percent = rssi_raw * 100;
    
    return rssi_percent;
}

/**
 * Чтение RSSI с усреднением
 * @param samples Количество выборок для усреднения
 * @return Усредненное значение RSSI
 */
uint8_t rx5808_read_rssi_averaged(uint8_t samples) {
    uint32_t rssi_sum = 0;
    
    for (int i = 0; i < samples; i++) {
        rssi_sum += rx5808_read_rssi();
        usleep(100);
    }
    
    return (uint8_t)(rssi_sum / samples);
}

/**
 * Проверка статуса модуля
 * @return 1 если модуль готов, 0 если нет
 */
int rx5808_is_ready(void) {
    if (!spi_initialized) return 0;
    
    // Проверка регистра статуса
    uint8_t status = rx5808_read_register(RX5808_REG_0F);
    return (status & 0x01) ? 1 : 0;
}

/**
 * Получение информации о модуле
 */
void rx5808_get_info(void) {
    if (!spi_initialized) {
        printf("❌ Модуль не инициализирован\n");
        return;
    }
    
    printf("📊 Информация о модуле RX5808:\n");
    printf("   Статус: %s\n", rx5808_is_ready() ? "Готов" : "Не готов");
    printf("   SPI FD: %d\n", spi_fd);
    printf("   Диапазон частот: %d-%d МГц\n", FREQ_MIN, FREQ_MAX);
    printf("   Текущий RSSI: %d%%\n", rx5808_read_rssi());
}

/**
 * Очистка ресурсов модуля
 * @return 0 при успехе
 */
int rx5808_cleanup(void) {
    printf("🧹 Очистка ресурсов RX5808...\n");
    
    if (spi_initialized) {
        // Отключение модуля
        rx5808_write_register(RX5808_REG_0C, 0x00);
        
        // Закрытие SPI
        if (spi_fd >= 0) {
            spiClose(spi_fd);
            spi_fd = -1;
        }
        
        spi_initialized = 0;
    }
    
    // Завершение pigpio
    gpioTerminate();
    
    printf("✅ Очистка завершена\n");
    return 0;
}
