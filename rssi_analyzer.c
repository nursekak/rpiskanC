#include "fpv_interceptor.h"
#include <math.h>

// Глобальные переменные для анализа RSSI
static uint8_t rssi_history[CHANNELS_COUNT][RSSI_SAMPLES];
static uint8_t rssi_index[CHANNELS_COUNT] = {0};
static uint8_t rssi_smoothed[CHANNELS_COUNT] = {0};
static uint32_t last_update[CHANNELS_COUNT] = {0};

/**
 * Инициализация анализатора RSSI
 * @return 0 при успехе, -1 при ошибке
 */
int rssi_analyzer_init(void) {
    printf("🔍 Инициализация анализатора RSSI...\n");
    
    // Очистка истории RSSI
    for (int i = 0; i < CHANNELS_COUNT; i++) {
        for (int j = 0; j < RSSI_SAMPLES; j++) {
            rssi_history[i][j] = 0;
        }
        rssi_index[i] = 0;
        rssi_smoothed[i] = 0;
        last_update[i] = 0;
    }
    
    printf("✅ Анализатор RSSI инициализирован\n");
    return 0;
}

/**
 * Сглаживание RSSI сигнала
 * @param frequency Частота в МГц
 * @param rssi Новое значение RSSI
 */
void smooth_rssi(uint16_t frequency, uint8_t rssi) {
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) return;
    
    int channel = frequency - FREQ_MIN;
    
    // Добавление нового значения в историю
    rssi_history[channel][rssi_index[channel]] = rssi;
    rssi_index[channel] = (rssi_index[channel] + 1) % RSSI_SAMPLES;
    
    // Расчет сглаженного значения
    uint32_t sum = 0;
    uint8_t count = 0;
    
    for (int i = 0; i < RSSI_SAMPLES; i++) {
        if (rssi_history[channel][i] > 0) {
            sum += rssi_history[channel][i];
            count++;
        }
    }
    
    if (count > 0) {
        rssi_smoothed[channel] = (uint8_t)(sum / count);
    }
    
    last_update[channel] = get_timestamp();
}

/**
 * Анализ RSSI для обнаружения сигнала
 * @param frequency Частота в МГц
 * @return Уровень RSSI (0-100)
 */
uint8_t analyze_rssi(uint16_t frequency) {
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) return 0;
    
    int channel = frequency - FREQ_MIN;
    
    // Чтение текущего RSSI (используем Linux GPIO версию)
    uint8_t current_rssi = rx5808_read_rssi_linux();
    
    // Сглаживание
    smooth_rssi(frequency, current_rssi);
    
    // Анализ тренда
    uint8_t trend = analyze_rssi_trend(channel);
    
    // Коррекция на основе тренда
    uint8_t corrected_rssi = current_rssi;
    if (trend > 50) {
        corrected_rssi = (current_rssi + rssi_smoothed[channel]) / 2;
    }
    
    return corrected_rssi;
}

/**
 * Анализ тренда RSSI
 * @param channel Номер канала
 * @return Тренд (0-100)
 */
uint8_t analyze_rssi_trend(int channel) {
    if (channel < 0 || channel >= CHANNELS_COUNT) return 0;
    
    uint8_t recent_sum = 0;
    uint8_t old_sum = 0;
    uint8_t recent_count = 0;
    uint8_t old_count = 0;
    
    // Анализ последних 20% выборок
    int recent_start = (RSSI_SAMPLES * 8) / 10;
    int old_start = 0;
    int old_end = RSSI_SAMPLES / 2;
    
    for (int i = recent_start; i < RSSI_SAMPLES; i++) {
        if (rssi_history[channel][i] > 0) {
            recent_sum += rssi_history[channel][i];
            recent_count++;
        }
    }
    
    for (int i = old_start; i < old_end; i++) {
        if (rssi_history[channel][i] > 0) {
            old_sum += rssi_history[channel][i];
            old_count++;
        }
    }
    
    if (recent_count == 0 || old_count == 0) return 50;
    
    uint8_t recent_avg = recent_sum / recent_count;
    uint8_t old_avg = old_sum / old_count;
    
    // Расчет тренда
    if (recent_avg > old_avg) {
        return 50 + ((recent_avg - old_avg) * 25) / 100;
    } else {
        return 50 - ((old_avg - recent_avg) * 25) / 100;
    }
}

/**
 * Обнаружение видеосигнала по RSSI
 * @param frequency Частота в МГц
 * @return 1 если видеосигнал обнаружен, 0 если нет
 */
uint8_t detect_video_signal(uint16_t frequency) {
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) return 0;
    
    int channel = frequency - FREQ_MIN;
    uint8_t rssi = analyze_rssi(frequency);
    
    // Проверка базового порога
    if (rssi < RSSI_THRESHOLD) return 0;
    
    // Анализ стабильности сигнала
    uint8_t stability = calculate_signal_stability(channel);
    
    // Анализ характеристик FPV сигнала
    uint8_t fpv_characteristics = analyze_fpv_characteristics(channel);
    
    // Комбинированная оценка
    uint8_t video_score = (rssi * 40 + stability * 30 + fpv_characteristics * 30) / 100;
    
    return (video_score > 60) ? 1 : 0;
}

/**
 * Расчет стабильности сигнала
 * @param channel Номер канала
 * @return Стабильность (0-100)
 */
uint8_t calculate_signal_stability(int channel) {
    if (channel < 0 || channel >= CHANNELS_COUNT) return 0;
    
    uint8_t min_rssi = 255;
    uint8_t max_rssi = 0;
    uint8_t valid_samples = 0;
    
    for (int i = 0; i < RSSI_SAMPLES; i++) {
        if (rssi_history[channel][i] > 0) {
            if (rssi_history[channel][i] < min_rssi) {
                min_rssi = rssi_history[channel][i];
            }
            if (rssi_history[channel][i] > max_rssi) {
                max_rssi = rssi_history[channel][i];
            }
            valid_samples++;
        }
    }
    
    if (valid_samples < 5) return 0;
    
    uint8_t range = max_rssi - min_rssi;
    uint8_t stability = (range < 20) ? 100 : (range < 40) ? 80 : (range < 60) ? 60 : 40;
    
    return stability;
}

/**
 * Анализ характеристик FPV сигнала
 * @param channel Номер канала
 * @return Оценка FPV характеристик (0-100)
 */
uint8_t analyze_fpv_characteristics(int channel) {
    if (channel < 0 || channel >= CHANNELS_COUNT) return 0;
    
    // Анализ периодичности (FPV имеет характерную периодичность)
    uint8_t periodicity = analyze_periodicity(channel);
    
    // Анализ амплитудной модуляции
    uint8_t amplitude_mod = analyze_amplitude_modulation(channel);
    
    // Анализ частотных характеристик
    uint8_t frequency_chars = analyze_frequency_characteristics(channel);
    
    return (periodicity + amplitude_mod + frequency_chars) / 3;
}

/**
 * Анализ периодичности сигнала
 * @param channel Номер канала
 * @return Периодичность (0-100)
 */
uint8_t analyze_periodicity(int channel) {
    if (channel < 0 || channel >= CHANNELS_COUNT) return 0;
    
    uint8_t peaks = 0;
    uint8_t valleys = 0;
    
    for (int i = 1; i < RSSI_SAMPLES - 1; i++) {
        if (rssi_history[channel][i] > 0) {
            uint8_t prev = rssi_history[channel][i-1];
            uint8_t curr = rssi_history[channel][i];
            uint8_t next = rssi_history[channel][i+1];
            
            if (curr > prev && curr > next) {
                peaks++;
            } else if (curr < prev && curr < next) {
                valleys++;
            }
        }
    }
    
    // FPV сигнал имеет характерную периодичность
    uint8_t periodicity_score = (peaks + valleys) * 10;
    return (periodicity_score > 100) ? 100 : periodicity_score;
}

/**
 * Анализ амплитудной модуляции
 * @param channel Номер канала
 * @return Оценка АМ (0-100)
 */
uint8_t analyze_amplitude_modulation(int channel) {
    if (channel < 0 || channel >= CHANNELS_COUNT) return 0;
    
    uint32_t sum = 0;
    uint8_t count = 0;
    uint8_t max_val = 0;
    uint8_t min_val = 255;
    
    for (int i = 0; i < RSSI_SAMPLES; i++) {
        if (rssi_history[channel][i] > 0) {
            sum += rssi_history[channel][i];
            count++;
            if (rssi_history[channel][i] > max_val) {
                max_val = rssi_history[channel][i];
            }
            if (rssi_history[channel][i] < min_val) {
                min_val = rssi_history[channel][i];
            }
        }
    }
    
    if (count < 5) return 0;
    
    uint8_t avg = sum / count;
    uint8_t modulation_depth = max_val - min_val;
    
    // FPV имеет характерную глубину модуляции
    uint8_t am_score = (modulation_depth > 20 && modulation_depth < 60) ? 80 : 
                       (modulation_depth > 10 && modulation_depth < 80) ? 60 : 40;
    
    return am_score;
}

/**
 * Анализ частотных характеристик
 * @param channel Номер канала
 * @return Оценка частотных характеристик (0-100)
 */
uint8_t analyze_frequency_characteristics(int channel) {
    if (channel < 0 || channel >= CHANNELS_COUNT) return 0;
    
    // Простой анализ изменений RSSI
    uint8_t changes = 0;
    uint8_t valid_samples = 0;
    
    for (int i = 1; i < RSSI_SAMPLES; i++) {
        if (rssi_history[channel][i] > 0 && rssi_history[channel][i-1] > 0) {
            uint8_t diff = abs(rssi_history[channel][i] - rssi_history[channel][i-1]);
            if (diff > 5) {
                changes++;
            }
            valid_samples++;
        }
    }
    
    if (valid_samples < 5) return 0;
    
    uint8_t change_rate = (changes * 100) / valid_samples;
    
    // FPV имеет характерную частоту изменений
    uint8_t freq_score = (change_rate > 20 && change_rate < 60) ? 80 :
                         (change_rate > 10 && change_rate < 80) ? 60 : 40;
    
    return freq_score;
}

/**
 * Получение статистики RSSI для канала
 * @param frequency Частота в МГц
 * @param stats Указатель на структуру статистики
 */
void get_rssi_stats(uint16_t frequency, rssi_stats_t *stats) {
    if (!stats || frequency < FREQ_MIN || frequency > FREQ_MAX) return;
    
    int channel = frequency - FREQ_MIN;
    
    stats->frequency = frequency;
    stats->current_rssi = rssi_smoothed[channel];
    stats->max_rssi = 0;
    stats->min_rssi = 255;
    stats->avg_rssi = 0;
    stats->samples = 0;
    
    uint32_t sum = 0;
    
    for (int i = 0; i < RSSI_SAMPLES; i++) {
        if (rssi_history[channel][i] > 0) {
            if (rssi_history[channel][i] > stats->max_rssi) {
                stats->max_rssi = rssi_history[channel][i];
            }
            if (rssi_history[channel][i] < stats->min_rssi) {
                stats->min_rssi = rssi_history[channel][i];
            }
            sum += rssi_history[channel][i];
            stats->samples++;
        }
    }
    
    if (stats->samples > 0) {
        stats->avg_rssi = sum / stats->samples;
    }
    
    stats->stability = calculate_signal_stability(channel);
    stats->last_update = last_update[channel];
}

/**
 * Очистка данных анализатора RSSI
 */
void rssi_analyzer_cleanup(void) {
    printf("🧹 Очистка анализатора RSSI...\n");
    
    for (int i = 0; i < CHANNELS_COUNT; i++) {
        for (int j = 0; j < RSSI_SAMPLES; j++) {
            rssi_history[i][j] = 0;
        }
        rssi_index[i] = 0;
        rssi_smoothed[i] = 0;
        last_update[i] = 0;
    }
    
    printf("✅ Анализатор RSSI очищен\n");
}

