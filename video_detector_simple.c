#include "fpv_interceptor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Глобальные переменные для видео
static int video_initialized = 0;
static char video_device[64] = "/dev/video0";

/**
 * Инициализация захвата видео (упрощенная версия без OpenCV)
 * @return 0 при успехе, -1 при ошибке
 */
int init_video_capture(void) {
    printf("📹 Инициализация захвата видео (упрощенная версия)...\n");
    printf("ℹ️ OpenCV недоступен, используется упрощенный режим\n");
    
    // Проверка доступности видеоустройства
    if (access(video_device, F_OK) != 0) {
        printf("❌ USB Video DVR не найден: %s\n", video_device);
        printf("ℹ️ Подключите USB Video DVR к Raspberry Pi\n");
        printf("ℹ️ Подключите аналоговый выход RX5808 к входу USB Video DVR\n");
        return -1;
    }
    
    video_initialized = 1;
    printf("✅ Видеозахват инициализирован (упрощенный режим)\n");
    return 0;
}

/**
 * Захват кадра видео (заглушка)
 * @param frequency Частота для которой захватывается видео
 * @return 0 при успехе, -1 при ошибке
 */
int capture_video_frame(uint16_t frequency) {
    if (!video_initialized) {
        printf("❌ Видео не инициализировано\n");
        return -1;
    }
    
    printf("📹 Захват кадра на частоте %d МГц (упрощенный режим)\n", frequency);
    
    // В упрощенном режиме просто логируем событие
    char filename[256];
    snprintf(filename, sizeof(filename), "capture_%d_%u.log", 
            frequency, get_timestamp());
    
    FILE *file = fopen(filename, "w");
    if (file) {
        fprintf(file, "FPV Interceptor - Захват кадра\n");
        fprintf(file, "Частота: %d МГц\n", frequency);
        fprintf(file, "Время: %u\n", get_timestamp());
        fprintf(file, "Режим: Упрощенный (без OpenCV)\n");
        fclose(file);
        printf("💾 Лог сохранен: %s\n", filename);
    }
    
    return 0;
}

/**
 * Сохранение видеопотока (заглушка)
 * @param frequency Частота для которой сохраняется видео
 */
void save_video_stream(uint16_t frequency) {
    if (!video_initialized) {
        printf("❌ Видео не инициализировано\n");
        return;
    }
    
    char filename[256];
    snprintf(filename, sizeof(filename), "video_%d_%u.log", 
            frequency, get_timestamp());
    
    printf("📹 Запись видеопотока: %s (упрощенный режим)\n", filename);
    
    FILE *file = fopen(filename, "w");
    if (file) {
        fprintf(file, "FPV Interceptor - Видеопоток\n");
        fprintf(file, "Частота: %d МГц\n", frequency);
        fprintf(file, "Время: %u\n", get_timestamp());
        fprintf(file, "Режим: Упрощенный (без OpenCV)\n");
        fprintf(file, "Примечание: Для полного видеозахвата установите OpenCV\n");
        fclose(file);
        printf("💾 Видеопоток сохранен: %s\n", filename);
    }
}

/**
 * Получение информации о видеоустройстве
 */
void get_video_info(void) {
    if (!video_initialized) {
        printf("❌ Видео не инициализировано\n");
        return;
    }
    
    printf("📊 Информация о видео (упрощенный режим):\n");
    printf("   Устройство: %s\n", video_device);
    printf("   Режим: Упрощенный (без OpenCV)\n");
    printf("   Статус: Доступен\n");
    printf("   Примечание: Для полного функционала установите OpenCV\n");
}

/**
 * Очистка ресурсов видео
 */
void video_detector_cleanup(void) {
    printf("🧹 Очистка ресурсов видео...\n");
    
    if (video_initialized) {
        video_initialized = 0;
    }
    
    printf("✅ Ресурсы видео очищены\n");
}
