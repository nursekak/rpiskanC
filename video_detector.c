#include "fpv_interceptor.h"
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// Глобальные переменные для видео
static cv::VideoCapture *video_capture = nullptr;
static cv::Mat current_frame;
static pthread_mutex_t frame_mutex = PTHREAD_MUTEX_INITIALIZER;
static int video_initialized = 0;
static char video_device[64] = "/dev/video0";
static int video_width = 640;
static int video_height = 480;
static int video_fps = 30;

// Функции для GUI
static void (*gui_update_callback)(cv::Mat*) = nullptr;
static void (*gui_status_callback)(const char*) = nullptr;

/**
 * Установка callback функций для GUI
 */
void set_gui_callbacks(void (*update_callback)(cv::Mat*), void (*status_callback)(const char*)) {
    gui_update_callback = update_callback;
    gui_status_callback = status_callback;
}

/**
 * Инициализация захвата видео с OpenCV
 * @return 0 при успехе, -1 при ошибке
 */
int init_video_capture(void) {
    printf("📹 Инициализация захвата видео с OpenCV...\n");
    
    // Проверка доступности видеоустройства
    if (access(video_device, F_OK) != 0) {
        printf("❌ USB Video DVR не найден: %s\n", video_device);
        printf("ℹ️ Подключите USB Video DVR к Raspberry Pi\n");
        printf("ℹ️ Подключите аналоговый выход RX5808 к входу USB Video DVR\n");
        
        if (gui_status_callback) {
            gui_status_callback("❌ USB Video DVR не найден");
        }
        return -1;
    }
    
    // Создание объекта захвата видео
    video_capture = new cv::VideoCapture();
    
    // Открытие видеоустройства
    if (!video_capture->open(video_device)) {
        printf("❌ Ошибка открытия видеоустройства: %s\n", video_device);
        delete video_capture;
        video_capture = nullptr;
        
        if (gui_status_callback) {
            gui_status_callback("❌ Ошибка открытия видео");
        }
        return -1;
    }
    
    // Настройка параметров видео
    video_capture->set(cv::CAP_PROP_FRAME_WIDTH, video_width);
    video_capture->set(cv::CAP_PROP_FRAME_HEIGHT, video_height);
    video_capture->set(cv::CAP_PROP_FPS, video_fps);
    video_capture->set(cv::CAP_PROP_BUFFERSIZE, 1); // Минимальная задержка
    
    // Проверка успешности настройки
    double actual_width = video_capture->get(cv::CAP_PROP_FRAME_WIDTH);
    double actual_height = video_capture->get(cv::CAP_PROP_FRAME_HEIGHT);
    double actual_fps = video_capture->get(cv::CAP_PROP_FPS);
    
    printf("✅ Видеозахват инициализирован:\n");
    printf("   Устройство: %s\n", video_device);
    printf("   Разрешение: %.0fx%.0f\n", actual_width, actual_height);
    printf("   FPS: %.1f\n", actual_fps);
    
    video_initialized = 1;
    
    if (gui_status_callback) {
        gui_status_callback("✅ Видеозахват готов");
    }
    
    return 0;
}

/**
 * Захват кадра видео
 * @param frequency Частота для которой захватывается видео
 * @return 0 при успехе, -1 при ошибке
 */
int capture_video_frame(uint16_t frequency) {
    if (!video_initialized || !video_capture) {
        printf("❌ Видео не инициализировано\n");
        return -1;
    }
    
    cv::Mat frame;
    
    // Захват кадра
    if (!video_capture->read(frame)) {
        printf("❌ Ошибка захвата кадра на частоте %d МГц\n", frequency);
        return -1;
    }
    
    // Проверка на пустой кадр
    if (frame.empty()) {
        printf("⚠️ Пустой кадр на частоте %d МГц\n", frequency);
        return -1;
    }
    
    // Блокировка мьютекса для обновления кадра
    pthread_mutex_lock(&frame_mutex);
    current_frame = frame.clone();
    pthread_mutex_unlock(&frame_mutex);
    
    // Обновление GUI
    if (gui_update_callback) {
        gui_update_callback(&current_frame);
    }
    
    printf("📹 Кадр захвачен на частоте %d МГц (%dx%d)\n", 
           frequency, frame.cols, frame.rows);
    
    return 0;
}

/**
 * Получение текущего кадра (для GUI)
 */
void* get_current_frame(void) {
    pthread_mutex_lock(&frame_mutex);
    cv::Mat *frame = new cv::Mat(current_frame);
    pthread_mutex_unlock(&frame_mutex);
    return static_cast<void*>(frame);
}

/**
 * Анализ качества видео
 * @param frame Кадр для анализа
 * @return Оценка качества (0-100)
 */
int analyze_video_quality(void* frame_ptr) {
    cv::Mat* frame = static_cast<cv::Mat*>(frame_ptr);
    if (!frame || frame->empty()) return 0;
    
    // Конвертация в оттенки серого
    cv::Mat gray;
    cv::cvtColor(*frame, gray, cv::COLOR_BGR2GRAY);
    
    // Вычисление лапласиана для оценки резкости
    cv::Mat laplacian;
    cv::Laplacian(gray, laplacian, CV_64F);
    
    cv::Scalar mean, stddev;
    cv::meanStdDev(laplacian, mean, stddev);
    
    // Оценка качества на основе дисперсии лапласиана
    double quality = stddev[0] * stddev[0];
    
    // Нормализация в диапазон 0-100
    int quality_score = (int)(std::min(100.0, quality / 10.0));
    
    return quality_score;
}

/**
 * Детекция движения в видео
 * @param frame Текущий кадр
 * @return 1 если движение обнаружено, 0 если нет
 */
int detect_motion(void* frame_ptr) {
    cv::Mat* frame = static_cast<cv::Mat*>(frame_ptr);
    if (!frame || frame->empty()) return 0;
    static cv::Mat prev_frame;
    static bool first_frame = true;
    
    // Конвертация в оттенки серого
    cv::Mat gray;
    cv::cvtColor(*frame, gray, cv::COLOR_BGR2GRAY);
    
    if (first_frame) {
        prev_frame = gray.clone();
        first_frame = false;
        return 0;
    }
    
    // Вычисление разности кадров
    cv::Mat diff;
    cv::absdiff(prev_frame, gray, diff);
    
    // Пороговая обработка
    cv::Mat thresh;
    cv::threshold(diff, thresh, 30, 255, cv::THRESH_BINARY);
    
    // Подсчет пикселей движения
    int motion_pixels = cv::countNonZero(thresh);
    int total_pixels = frame->rows * frame->cols;
    double motion_ratio = (double)motion_pixels / total_pixels;
    
    // Обновление предыдущего кадра
    prev_frame = gray.clone();
    
    // Детекция движения при соотношении > 0.01 (1%)
    return (motion_ratio > 0.01) ? 1 : 0;
}

/**
 * Сохранение видеопотока
 * @param frequency Частота для которой сохраняется видео
 */
void save_video_stream(uint16_t frequency) {
    if (!video_initialized || !video_capture) {
        printf("❌ Видео не инициализировано\n");
        return;
    }
    
    char filename[256];
    snprintf(filename, sizeof(filename), "captures/video_%d_%u.avi", 
            frequency, get_timestamp());
    
    printf("📹 Запись видеопотока: %s\n", filename);
    
    // Создание директории если не существует
    system("mkdir -p captures");
    
    // Настройка кодека для записи
    int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    cv::VideoWriter writer(filename, fourcc, video_fps, 
                          cv::Size(video_width, video_height));
    
    if (!writer.isOpened()) {
        printf("❌ Ошибка создания файла записи: %s\n", filename);
        return;
    }
    
    // Запись 30 секунд видео
    int frame_count = 0;
    int max_frames = video_fps * 30; // 30 секунд
    
    while (frame_count < max_frames) {
        cv::Mat frame;
        if (video_capture->read(frame) && !frame.empty()) {
            writer.write(frame);
            frame_count++;
            
            // Обновление GUI
            if (gui_update_callback) {
                gui_update_callback(&frame);
            }
            
            // Небольшая задержка
            usleep(1000000 / video_fps); // 1/FPS секунды
        } else {
            break;
        }
    }
    
    writer.release();
    printf("💾 Видеопоток сохранен: %s (%d кадров)\n", filename, frame_count);
    
    if (gui_status_callback) {
        char status[256];
        snprintf(status, sizeof(status), "💾 Видео сохранено: %d кадров", frame_count);
        gui_status_callback(status);
    }
}

/**
 * Получение информации о видеоустройстве
 */
void get_video_info(void) {
    if (!video_initialized || !video_capture) {
        printf("❌ Видео не инициализировано\n");
        return;
    }
    
    double width = video_capture->get(cv::CAP_PROP_FRAME_WIDTH);
    double height = video_capture->get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = video_capture->get(cv::CAP_PROP_FPS);
    double brightness = video_capture->get(cv::CAP_PROP_BRIGHTNESS);
    double contrast = video_capture->get(cv::CAP_PROP_CONTRAST);
    
    printf("📊 Информация о видео:\n");
    printf("   Устройство: %s\n", video_device);
    printf("   Разрешение: %.0fx%.0f\n", width, height);
    printf("   FPS: %.1f\n", fps);
    printf("   Яркость: %.1f\n", brightness);
    printf("   Контраст: %.1f\n", contrast);
    printf("   Статус: Активен\n");
}

/**
 * Настройка параметров видео
 * @param width Ширина кадра
 * @param height Высота кадра
 * @param fps Частота кадров
 */
void set_video_parameters(int width, int height, int fps) {
    video_width = width;
    video_height = height;
    video_fps = fps;
    
    if (video_initialized && video_capture) {
        video_capture->set(cv::CAP_PROP_FRAME_WIDTH, width);
        video_capture->set(cv::CAP_PROP_FRAME_HEIGHT, height);
        video_capture->set(cv::CAP_PROP_FPS, fps);
        
        printf("📹 Параметры видео обновлены: %dx%d @ %d FPS\n", width, height, fps);
    }
}

/**
 * Очистка ресурсов видео
 */
void video_detector_cleanup(void) {
    printf("🧹 Очистка ресурсов видео...\n");
    
    if (video_capture) {
        video_capture->release();
        delete video_capture;
        video_capture = nullptr;
    }
    
    video_initialized = 0;
    pthread_mutex_destroy(&frame_mutex);
    
    printf("✅ Ресурсы видео очищены\n");
}
