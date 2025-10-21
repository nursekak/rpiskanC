#include "fpv_interceptor.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

// Глобальные переменные для видео
static VideoCapture video_cap;
static int video_initialized = 0;
static char video_device[64] = "/dev/video0";

/**
 * Инициализация захвата видео
 * @return 0 при успехе, -1 при ошибке
 */
int init_video_capture(void) {
    printf("📹 Инициализация захвата видео...\n");
    printf("ℹ️ Ожидается USB Video DVR для оцифровки аналогового сигнала\n");
    
    // Попытка открыть видеоустройство
    video_cap.open(video_device);
    
    if (!video_cap.isOpened()) {
        printf("❌ Не удалось открыть видеоустройство %s\n", video_device);
        printf("ℹ️ Убедитесь что USB Video DVR подключен и работает\n");
        
        // Попытка найти альтернативное устройство
        for (int i = 1; i < 5; i++) {
            snprintf(video_device, sizeof(video_device), "/dev/video%d", i);
            video_cap.open(video_device);
            if (video_cap.isOpened()) {
                printf("✅ Найдено видеоустройство: %s\n", video_device);
                break;
            }
        }
        
        if (!video_cap.isOpened()) {
            printf("❌ USB Video DVR не найден\n");
            printf("ℹ️ Подключите USB Video DVR к Raspberry Pi\n");
            printf("ℹ️ Подключите аналоговый выход RX5808 к входу USB Video DVR\n");
            return -1;
        }
    }
    
    // Настройка параметров захвата
    video_cap.set(CAP_PROP_FRAME_WIDTH, VIDEO_WIDTH);
    video_cap.set(CAP_PROP_FRAME_HEIGHT, VIDEO_HEIGHT);
    video_cap.set(CAP_PROP_FPS, VIDEO_FPS);
    
    // Проверка настроек
    int width = (int)video_cap.get(CAP_PROP_FRAME_WIDTH);
    int height = (int)video_cap.get(CAP_PROP_FRAME_HEIGHT);
    int fps = (int)video_cap.get(CAP_PROP_FPS);
    
    printf("📊 Параметры видео: %dx%d @ %d FPS\n", width, height, fps);
    
    video_initialized = 1;
    printf("✅ Захват видео инициализирован\n");
    
    return 0;
}

/**
 * Захват кадра видео
 * @param frequency Частота для которой захватывается видео
 * @return 0 при успехе, -1 при ошибке
 */
int capture_video_frame(uint16_t frequency) {
    if (!video_initialized) {
        printf("❌ Видео не инициализировано\n");
        return -1;
    }
    
    Mat frame;
    
    // Захват кадра
    if (!video_cap.read(frame)) {
        printf("❌ Ошибка захвата кадра\n");
        return -1;
    }
    
    if (frame.empty()) {
        printf("❌ Пустой кадр\n");
        return -1;
    }
    
    // Анализ кадра на наличие видеосигнала
    uint8_t video_detected = analyze_video_frame(frame);
    
    if (video_detected) {
        printf("📺 Видеосигнал обнаружен на частоте %d МГц\n", frequency);
        
        // Сохранение кадра
        char filename[256];
        snprintf(filename, sizeof(filename), "capture_%d_%ld.jpg", 
                frequency, get_timestamp());
        imwrite(filename, frame);
        
        printf("💾 Кадр сохранен: %s\n", filename);
    }
    
    return 0;
}

/**
 * Анализ кадра на наличие видеосигнала
 * @param frame Кадр для анализа
 * @return 1 если видеосигнал обнаружен, 0 если нет
 */
uint8_t analyze_video_frame(Mat &frame) {
    if (frame.empty()) return 0;
    
    // Конвертация в оттенки серого
    Mat gray;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    
    // Анализ яркости
    uint8_t brightness_score = analyze_brightness(gray);
    
    // Анализ контраста
    uint8_t contrast_score = analyze_contrast(gray);
    
    // Анализ движения
    uint8_t motion_score = analyze_motion(gray);
    
    // Анализ синхронизации
    uint8_t sync_score = analyze_sync_signals(gray);
    
    // Комбинированная оценка
    uint8_t video_score = (brightness_score * 25 + contrast_score * 25 + 
                          motion_score * 25 + sync_score * 25) / 100;
    
    return (video_score > 50) ? 1 : 0;
}

/**
 * Анализ яркости кадра
 * @param gray Кадр в оттенках серого
 * @return Оценка яркости (0-100)
 */
uint8_t analyze_brightness(Mat &gray) {
    Scalar mean_brightness = mean(gray);
    double brightness = mean_brightness[0];
    
    // FPV видео имеет характерную яркость
    if (brightness > 30 && brightness < 200) {
        return 80;
    } else if (brightness > 10 && brightness < 250) {
        return 60;
    } else {
        return 20;
    }
}

/**
 * Анализ контраста кадра
 * @param gray Кадр в оттенках серого
 * @return Оценка контраста (0-100)
 */
uint8_t analyze_contrast(Mat &gray) {
    Mat hist;
    int histSize = 256;
    float range[] = {0, 256};
    const float* histRange = {range};
    
    calcHist(&gray, 1, 0, Mat(), hist, 1, &histSize, &histRange);
    
    // Поиск пиков в гистограмме
    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(hist, &minVal, &maxVal, &minLoc, &maxLoc);
    
    // Анализ распределения яркости
    uint8_t contrast_score = 0;
    
    // Подсчет пикселей в разных диапазонах яркости
    int dark_pixels = 0, mid_pixels = 0, bright_pixels = 0;
    
    for (int i = 0; i < gray.rows; i++) {
        for (int j = 0; j < gray.cols; j++) {
            uint8_t pixel = gray.at<uint8_t>(i, j);
            if (pixel < 85) dark_pixels++;
            else if (pixel < 170) mid_pixels++;
            else bright_pixels++;
        }
    }
    
    int total_pixels = gray.rows * gray.cols;
    
    // FPV видео имеет характерное распределение контраста
    if (dark_pixels > total_pixels * 0.1 && bright_pixels > total_pixels * 0.1) {
        contrast_score = 80;
    } else if (mid_pixels > total_pixels * 0.3) {
        contrast_score = 60;
    } else {
        contrast_score = 30;
    }
    
    return contrast_score;
}

/**
 * Анализ движения в кадре
 * @param gray Кадр в оттенках серого
 * @return Оценка движения (0-100)
 */
uint8_t analyze_motion(Mat &gray) {
    static Mat prev_frame;
    static int first_frame = 1;
    
    if (first_frame) {
        gray.copyTo(prev_frame);
        first_frame = 0;
        return 50; // Средняя оценка для первого кадра
    }
    
    // Вычисление разности кадров
    Mat diff;
    absdiff(prev_frame, gray, diff);
    
    // Пороговая обработка
    Mat thresh;
    threshold(diff, thresh, 30, 255, THRESH_BINARY);
    
    // Подсчет изменений
    int changed_pixels = countNonZero(thresh);
    int total_pixels = gray.rows * gray.cols;
    double motion_ratio = (double)changed_pixels / total_pixels;
    
    // Обновление предыдущего кадра
    gray.copyTo(prev_frame);
    
    // FPV видео имеет характерное движение
    uint8_t motion_score = 0;
    if (motion_ratio > 0.01 && motion_ratio < 0.3) {
        motion_score = 80;
    } else if (motion_ratio > 0.005 && motion_ratio < 0.5) {
        motion_score = 60;
    } else if (motion_ratio > 0.001) {
        motion_score = 40;
    }
    
    return motion_score;
}

/**
 * Анализ синхросигналов
 * @param gray Кадр в оттенках серого
 * @return Оценка синхросигналов (0-100)
 */
uint8_t analyze_sync_signals(Mat &gray) {
    // Анализ горизонтальных линий (горизонтальная синхронизация)
    uint8_t h_sync_score = analyze_horizontal_sync(gray);
    
    // Анализ вертикальных линий (вертикальная синхронизация)
    uint8_t v_sync_score = analyze_vertical_sync(gray);
    
    return (h_sync_score + v_sync_score) / 2;
}

/**
 * Анализ горизонтальной синхронизации
 * @param gray Кадр в оттенках серого
 * @return Оценка ГС (0-100)
 */
uint8_t analyze_horizontal_sync(Mat &gray) {
    // Поиск горизонтальных линий
    Mat sobel_x;
    Sobel(gray, sobel_x, CV_64F, 1, 0, 3);
    
    // Анализ горизонтальных градиентов
    int horizontal_lines = 0;
    for (int i = 0; i < sobel_x.rows; i++) {
        double row_sum = 0;
        for (int j = 0; j < sobel_x.cols; j++) {
            row_sum += abs(sobel_x.at<double>(i, j));
        }
        if (row_sum > sobel_x.cols * 10) { // Порог для горизонтальных линий
            horizontal_lines++;
        }
    }
    
    // FPV видео имеет характерные горизонтальные синхросигналы
    uint8_t h_sync_score = (horizontal_lines > 5) ? 80 : 
                          (horizontal_lines > 2) ? 60 : 30;
    
    return h_sync_score;
}

/**
 * Анализ вертикальной синхронизации
 * @param gray Кадр в оттенках серого
 * @return Оценка ВС (0-100)
 */
uint8_t analyze_vertical_sync(Mat &gray) {
    // Поиск вертикальных линий
    Mat sobel_y;
    Sobel(gray, sobel_y, CV_64F, 0, 1, 3);
    
    // Анализ вертикальных градиентов
    int vertical_lines = 0;
    for (int j = 0; j < sobel_y.cols; j++) {
        double col_sum = 0;
        for (int i = 0; i < sobel_y.rows; i++) {
            col_sum += abs(sobel_y.at<double>(i, j));
        }
        if (col_sum > sobel_y.rows * 10) { // Порог для вертикальных линий
            vertical_lines++;
        }
    }
    
    // FPV видео имеет характерные вертикальные синхросигналы
    uint8_t v_sync_score = (vertical_lines > 3) ? 80 : 
                          (vertical_lines > 1) ? 60 : 30;
    
    return v_sync_score;
}

/**
 * Сохранение видеопотока
 * @param frequency Частота для которой сохраняется видео
 */
void save_video_stream(uint16_t frequency) {
    if (!video_initialized) {
        printf("❌ Видео не инициализировано\n");
        return;
    }
    
    char filename[256];
    snprintf(filename, sizeof(filename), "video_%d_%ld.avi", 
            frequency, get_timestamp());
    
    // Создание видеописателя
    VideoWriter writer(filename, VideoWriter::fourcc('M','J','P','G'), 
                      VIDEO_FPS, Size(VIDEO_WIDTH, VIDEO_HEIGHT));
    
    if (!writer.isOpened()) {
        printf("❌ Ошибка создания видеофайла: %s\n", filename);
        return;
    }
    
    printf("📹 Начало записи видео: %s\n", filename);
    
    Mat frame;
    int frame_count = 0;
    int max_frames = VIDEO_FPS * 10; // 10 секунд
    
    while (frame_count < max_frames && running) {
        if (video_cap.read(frame)) {
            writer.write(frame);
            frame_count++;
            
            if (frame_count % VIDEO_FPS == 0) {
                printf("📹 Записано %d секунд\n", frame_count / VIDEO_FPS);
            }
        } else {
            printf("❌ Ошибка чтения кадра\n");
            break;
        }
        
        usleep(1000000 / VIDEO_FPS); // Задержка для правильного FPS
    }
    
    writer.release();
    printf("✅ Видео сохранено: %s (%d кадров)\n", filename, frame_count);
}

/**
 * Получение информации о видеоустройстве
 */
void get_video_info(void) {
    if (!video_initialized) {
        printf("❌ Видео не инициализировано\n");
        return;
    }
    
    printf("📊 Информация о видео:\n");
    printf("   Устройство: %s\n", video_device);
    printf("   Разрешение: %dx%d\n", 
           (int)video_cap.get(CAP_PROP_FRAME_WIDTH),
           (int)video_cap.get(CAP_PROP_FRAME_HEIGHT));
    printf("   FPS: %d\n", (int)video_cap.get(CAP_PROP_FPS));
    printf("   Формат: %d\n", (int)video_cap.get(CAP_PROP_FOURCC));
}

/**
 * Очистка ресурсов видео
 */
void video_detector_cleanup(void) {
    printf("🧹 Очистка ресурсов видео...\n");
    
    if (video_initialized) {
        video_cap.release();
        video_initialized = 0;
    }
    
    printf("✅ Ресурсы видео очищены\n");
}

