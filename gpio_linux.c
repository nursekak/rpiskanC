#include "fpv_interceptor.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <string.h>

// Глобальные переменные для GPIO и SPI
static int spi_fd = -1;
static int spi_initialized = 0;

/**
 * Экспорт GPIO пина
 */
int gpio_export(int pin) {
    char buffer[64];
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) return -1;
    
    snprintf(buffer, sizeof(buffer), "%d", pin);
    write(fd, buffer, strlen(buffer));
    close(fd);
    
    return 0;
}

/**
 * Настройка направления GPIO
 */
int gpio_direction(int pin, int direction) {
    char path[64];
    int fd;
    
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    
    write(fd, direction ? "out" : "in", direction ? 3 : 2);
    close(fd);
    
    return 0;
}

/**
 * Чтение GPIO пина
 */
int gpio_read(int pin) {
    char path[64];
    char value[3];
    int fd;
    
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    
    read(fd, value, 3);
    close(fd);
    
    return atoi(value);
}

/**
 * Запись в GPIO пин
 */
int gpio_write(int pin, int value) {
    char path[64];
    int fd;
    
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    
    write(fd, value ? "1" : "0", 1);
    close(fd);
    
    return 0;
}

/**
 * Инициализация GPIO
 */
int gpio_init(void) {
    printf("🔧 Инициализация GPIO...\n");
    
    // Экспорт пинов
    gpio_export(CS_PIN);
    gpio_export(MOSI_PIN);
    gpio_export(SCK_PIN);
    gpio_export(RSSI_PIN);
    
    // Настройка направлений
    gpio_direction(CS_PIN, 1);    // Выход
    gpio_direction(MOSI_PIN, 1);  // Выход
    gpio_direction(SCK_PIN, 1);   // Выход
    gpio_direction(RSSI_PIN, 0);  // Вход
    
    // Установка начальных состояний
    gpio_write(CS_PIN, 1);
    gpio_write(MOSI_PIN, 0);
    gpio_write(SCK_PIN, 0);
    
    printf("✅ GPIO инициализирован\n");
    return 0;
}

/**
 * Инициализация SPI
 */
int spi_init(void) {
    printf("📡 Инициализация SPI...\n");
    
    spi_fd = open("/dev/spidev0.0", O_RDWR);
    if (spi_fd < 0) {
        printf("❌ Ошибка открытия SPI устройства\n");
        return -1;
    }
    
    // Настройка SPI режима
    uint8_t mode = SPI_MODE_0;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0) {
        printf("❌ Ошибка настройки SPI режима\n");
        close(spi_fd);
        return -1;
    }
    
    // Настройка скорости
    uint32_t speed = 2000000; // 2 MHz
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        printf("❌ Ошибка настройки SPI скорости\n");
        close(spi_fd);
        return -1;
    }
    
    spi_initialized = 1;
    printf("✅ SPI инициализирован\n");
    return 0;
}

/**
 * SPI передача
 */
int spi_transfer(uint8_t *tx_buf, uint8_t *rx_buf, int len) {
    struct spi_ioc_transfer transfer = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = len,
        .speed_hz = 2000000,
        .bits_per_word = 8,
    };
    
    return ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer);
}

/**
 * Инициализация RX5808 с Linux GPIO
 */
int rx5808_init_linux(void) {
    printf("🔧 Инициализация RX5808 (Linux GPIO)...\n");
    
    if (gpio_init() != 0) {
        printf("❌ Ошибка инициализации GPIO\n");
        return -1;
    }
    
    if (spi_init() != 0) {
        printf("❌ Ошибка инициализации SPI\n");
        return -1;
    }
    
    printf("✅ RX5808 инициализирован\n");
    return 0;
}

/**
 * Запись в регистр RX5808
 */
void rx5808_write_register_linux(uint8_t reg, uint8_t data) {
    if (!spi_initialized) return;
    
    uint8_t tx_data[2];
    uint8_t rx_data[2];
    
    tx_data[0] = RX5808_CMD_WRITE | reg;
    tx_data[1] = data;
    
    gpio_write(CS_PIN, 0);
    spi_transfer(tx_data, rx_data, 2);
    gpio_write(CS_PIN, 1);
    
    usleep(10);
}

/**
 * Чтение регистра RX5808
 */
uint8_t rx5808_read_register_linux(uint8_t reg) {
    if (!spi_initialized) return 0;
    
    uint8_t tx_data[2];
    uint8_t rx_data[2];
    
    tx_data[0] = RX5808_CMD_READ | reg;
    tx_data[1] = 0x00;
    
    gpio_write(CS_PIN, 0);
    spi_transfer(tx_data, rx_data, 2);
    gpio_write(CS_PIN, 1);
    
    usleep(10);
    
    return rx_data[1];
}

/**
 * Установка частоты
 */
int rx5808_set_frequency_linux(uint16_t frequency) {
    if (!spi_initialized) {
        printf("❌ SPI не инициализирован\n");
        return -1;
    }
    
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) {
        printf("❌ Частота %d МГц вне диапазона\n", frequency);
        return -1;
    }
    
    printf("📡 Установка частоты: %d МГц\n", frequency);
    
    // Расчет параметров для RX5808
    uint32_t freq_reg = (frequency - 479) * 8192 / 1000;
    
    // Разбивка на байты
    uint8_t reg_0A = (freq_reg >> 8) & 0xFF;
    uint8_t reg_0B = freq_reg & 0xFF;
    
    // Запись в регистры
    rx5808_write_register_linux(RX5808_REG_0A, reg_0A);
    rx5808_write_register_linux(RX5808_REG_0B, reg_0B);
    
    // Включение синтезатора
    rx5808_write_register_linux(RX5808_REG_0C, 0x01);
    
    // Ожидание стабилизации
    usleep(50000);
    
    printf("✅ Частота установлена: %d МГц\n", frequency);
    return 0;
}

/**
 * Чтение RSSI
 */
uint8_t rx5808_read_rssi_linux(void) {
    if (!spi_initialized) return 0;
    
    // Чтение GPIO пина RSSI
    int rssi_digital = gpio_read(RSSI_PIN);
    
    // Простое преобразование в проценты
    uint8_t rssi_percent = rssi_digital * 100;
    
    return rssi_percent;
}

/**
 * Очистка ресурсов
 */
int rx5808_cleanup_linux(void) {
    printf("🧹 Очистка ресурсов RX5808 (Linux)...\n");
    
    if (spi_initialized) {
        // Отключение модуля
        rx5808_write_register_linux(RX5808_REG_0C, 0x00);
        
        // Закрытие SPI
        if (spi_fd >= 0) {
            close(spi_fd);
            spi_fd = -1;
        }
        
        spi_initialized = 0;
    }
    
    printf("✅ Очистка завершена\n");
    return 0;
}
