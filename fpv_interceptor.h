#ifndef FPV_INTERCEPTOR_H
#define FPV_INTERCEPTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>

// Конфигурация GPIO пинов для Raspberry Pi
#define CS_PIN          8    // CH2 - Chip Select
#define MOSI_PIN        10   // A - MOSI
#define MISO_PIN        9    // 6.5M - MISO  
#define SCK_PIN         11   // CH1 - SCK
#define RSSI_PIN        7    // RSSI - Signal Strength

// Конфигурация частот 5.8 ГГц
#define FREQ_MIN        5725    // Минимальная частота в МГц
#define FREQ_MAX        6000     // Максимальная частота в МГц
#define FREQ_STEP       1        // Шаг сканирования в МГц
#define CHANNELS_COUNT  (FREQ_MAX - FREQ_MIN + 1)

// Конфигурация RSSI
#define RSSI_THRESHOLD      50      // Порог RSSI для обнаружения сигнала
#define RSSI_SAMPLES        100     // Количество выборок для анализа
#define RSSI_SMOOTHING      10      // Сглаживание RSSI

// Конфигурация видео
#define VIDEO_WIDTH         640
#define VIDEO_HEIGHT        480
#define VIDEO_FPS           30
#define VIDEO_BUFFER_SIZE   1024

// Конфигурация сканирования
#define SCAN_DWELL_TIME     100     // Время задержки на частоте (мс)
#define SCAN_TIMEOUT        5000    // Таймаут сканирования (мс)
#define MAX_SIGNALS         10      // Максимальное количество найденных сигналов

// Структуры данных
typedef struct {
    uint16_t frequency;     // Частота в МГц
    uint8_t rssi;          // Уровень сигнала (0-100)
    uint8_t video_detected; // Обнаружен ли видеосигнал
    uint32_t timestamp;     // Время обнаружения
} signal_info_t;

typedef struct {
    uint16_t frequency;
    uint8_t current_rssi;
    uint8_t max_rssi;
    uint8_t min_rssi;
    uint8_t avg_rssi;
    uint8_t samples;
    uint8_t stability;
    uint32_t last_update;
} rssi_stats_t;

typedef struct {
    uint16_t frequency;
    uint8_t rssi;
    uint8_t video_detected;
    uint32_t timestamp;
    char video_file[256];
} detected_signal_t;

typedef struct {
    uint8_t rssi_history[RSSI_SAMPLES];
    uint8_t rssi_index;
    uint8_t rssi_smoothed;
    uint8_t video_detected;
    uint32_t last_update;
} channel_data_t;

// Глобальные переменные
extern channel_data_t channels[CHANNELS_COUNT];
extern detected_signal_t detected_signals[MAX_SIGNALS];
extern uint8_t signal_count;
extern volatile int running;

// Функции драйвера RX5808
int rx5808_init(void);
void rx5808_reset(void);
void rx5808_write_register(uint8_t reg, uint8_t data);
uint8_t rx5808_read_register(uint8_t reg);
int rx5808_set_frequency(uint16_t frequency);
uint8_t rx5808_read_rssi(void);
uint8_t rx5808_read_rssi_averaged(uint8_t samples);
int rx5808_is_ready(void);
void rx5808_get_info(void);
int rx5808_cleanup(void);

// Функции анализа RSSI
int rssi_analyzer_init(void);
uint8_t analyze_rssi(uint16_t frequency);
uint8_t detect_video_signal(uint16_t frequency);
void smooth_rssi(uint16_t frequency, uint8_t rssi);
uint8_t analyze_rssi_trend(int channel);
uint8_t calculate_signal_stability(int channel);
uint8_t analyze_fpv_characteristics(int channel);
uint8_t analyze_periodicity(int channel);
uint8_t analyze_amplitude_modulation(int channel);
uint8_t analyze_frequency_characteristics(int channel);
void get_rssi_stats(uint16_t frequency, rssi_stats_t *stats);
void rssi_analyzer_cleanup(void);

// Функции сканирования
int frequency_scanner_init(void);
int scan_frequency_range(void);
int scan_single_frequency(uint16_t frequency);
void scan_continuous(void);
void* scan_thread_function(void* arg);
int start_continuous_scan(void);
int stop_continuous_scan(void);
int scan_frequency_range_custom(uint16_t start_freq, uint16_t end_freq);
int monitor_frequency(uint16_t frequency, uint32_t duration);
void get_scan_stats(void);
void frequency_scanner_cleanup(void);

// Функции обнаружения сигналов
int add_detected_signal(uint16_t frequency, uint8_t rssi, uint8_t video);
void print_detected_signals(void);
void save_signal_data(void);

// Функции видео
int init_video_capture(void);
int capture_video_frame(uint16_t frequency);
void save_video_stream(uint16_t frequency);
uint8_t analyze_video_frame(void* frame);
uint8_t analyze_brightness(void* gray);
uint8_t analyze_contrast(void* gray);
uint8_t analyze_motion(void* gray);
uint8_t analyze_sync_signals(void* gray);
uint8_t analyze_horizontal_sync(void* gray);
uint8_t analyze_vertical_sync(void* gray);
void get_video_info(void);
void video_detector_cleanup(void);

// Утилиты
uint32_t get_timestamp(void);
void print_status(uint16_t frequency, uint8_t rssi);
void signal_handler(int sig);
void cleanup_resources(void);

// Константы для RX5808
#define RX5808_REG_0A    0x0A
#define RX5808_REG_0B    0x0B
#define RX5808_REG_0C    0x0C
#define RX5808_REG_0D    0x0D
#define RX5808_REG_0E    0x0E
#define RX5808_REG_0F    0x0F

// Команды RX5808
#define RX5808_CMD_READ   0x00
#define RX5808_CMD_WRITE  0x80

#endif // FPV_INTERCEPTOR_H
