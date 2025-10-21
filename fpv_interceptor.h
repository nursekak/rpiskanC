#ifndef FPV_INTERCEPTOR_H
#define FPV_INTERCEPTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

// Конфигурация частот
#define FREQ_MIN 5725    // Минимальная частота (МГц)
#define FREQ_MAX 6000    // Максимальная частота (МГц)
#define FREQ_STEP 1      // Шаг сканирования (МГц)

// RSSI настройки
#define RSSI_THRESHOLD 50    // Порог RSSI для детекции сигнала
#define RSSI_SAMPLES 100     // Количество образцов RSSI для анализа
#define RSSI_HISTORY_SIZE 50 // Размер истории RSSI
#define CHANNELS_COUNT (FREQ_MAX - FREQ_MIN + 1) // Количество каналов

// GPIO пины для RX5808
#define CS_PIN 8      // Chip Select
#define MOSI_PIN 10   // Master Out Slave In
#define MISO_PIN 9    // Master In Slave Out
#define SCK_PIN 11    // Serial Clock
#define RSSI_PIN 7    // RSSI Input

// RX5808 команды
#define RX5808_CMD_READ  0x00
#define RX5808_CMD_WRITE 0x80

// Структуры данных
typedef struct {
    uint16_t frequency;    // Частота в МГц
    uint8_t rssi;          // Уровень RSSI (0-100)
    uint32_t timestamp;    // Временная метка
    char signal_type[32];  // Тип сигнала
    int quality;          // Качество сигнала (0-100)
} signal_info_t;

typedef struct {
    uint16_t frequency;    // Частота
    uint8_t current_rssi;  // Текущий RSSI
    uint8_t max_rssi;      // Максимальный RSSI
    uint8_t min_rssi;      // Минимальный RSSI
    uint8_t avg_rssi;      // Средний RSSI
    int samples;           // Количество образцов
    uint8_t stability;     // Стабильность сигнала
    uint32_t last_update;  // Время последнего обновления
} rssi_stats_t;

typedef struct {
    uint16_t frequency;
    uint8_t rssi;
    uint32_t timestamp;
    char filename[256];
    int video_quality;
    int motion_detected;
} detected_signal_t;

// Глобальные переменные
extern detected_signal_t detected_signals[100];
extern int detected_count;

// Функции RX5808 драйвера (заглушки для GUI)
int rx5808_init(void);
int rx5808_reset(void);
void rx5808_write_register(uint8_t reg, uint8_t data);
uint8_t rx5808_read_register(uint8_t reg);
int rx5808_set_frequency(uint16_t frequency);
uint8_t rx5808_read_rssi(void);
uint8_t rx5808_read_rssi_averaged(int samples);
void rx5808_get_info(void);
void rx5808_cleanup(void);

// Функции анализатора RSSI
int rssi_analyzer_init(void);
uint8_t analyze_rssi(uint16_t frequency);
void get_rssi_stats(uint16_t frequency, rssi_stats_t *stats);
int detect_signal_presence(uint16_t frequency);
uint8_t analyze_amplitude_modulation(int channel);
void rssi_analyzer_cleanup(void);

// Функции видеодетектора
int init_video_capture(void);
int capture_video_frame(uint16_t frequency);
void save_video_stream(uint16_t frequency);
void get_video_info(void);
void set_gui_callbacks(void (*update_callback)(void*), void (*status_callback)(const char*));
void* get_current_frame(void);
int analyze_video_quality(void* frame);
int detect_motion(void* frame);
void set_video_parameters(int width, int height, int fps);
void video_detector_cleanup(void);
int test_usb_dvr(void);

// Функции частотного сканера
int frequency_scanner_init(void);
int scan_frequency_range(uint16_t start_freq, uint16_t end_freq, int dwell_time);
int monitor_frequency(uint16_t frequency, int timeout_ms);
int auto_scan_for_signals(void);
void frequency_scanner_cleanup(void);

// Утилиты
uint32_t get_timestamp(void);
void add_detected_signal(uint16_t frequency, uint8_t rssi, const char* signal_type);
void print_detected_signals(void);
void save_signal_data(void);
void cleanup_resources(void);

// GUI функции
void update_status(const char *message);
void update_rssi_display(uint8_t rssi, uint16_t frequency);
void* convert_mat_to_pixbuf(void* mat);
int init_gui_video_capture(void);

#ifdef __cplusplus
}
#endif

#endif // FPV_INTERCEPTOR_H