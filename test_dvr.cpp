#include "fpv_interceptor.h"
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <iostream>

int main() {
    printf("🧪 Тестирование USB DVR...\n");
    
    // Тест 1: Проверка доступности устройства
    printf("\n1️⃣ Проверка доступности /dev/video0...\n");
    if (access("/dev/video0", F_OK) != 0) {
        printf("❌ /dev/video0 не найден\n");
        return -1;
    }
    printf("✅ /dev/video0 найден\n");
    
    // Тест 2: Открытие устройства
    printf("\n2️⃣ Открытие USB DVR...\n");
    cv::VideoCapture cap;
    if (!cap.open("/dev/video0")) {
        printf("❌ Не удалось открыть /dev/video0\n");
        return -1;
    }
    printf("✅ USB DVR открыт\n");
    
    // Тест 3: Настройка параметров
    printf("\n3️⃣ Настройка параметров...\n");
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(cv::CAP_PROP_FPS, 30);
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
    
    // Проверка настроек
    double width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    double fourcc = cap.get(cv::CAP_PROP_FOURCC);
    
    printf("   Разрешение: %.0fx%.0f\n", width, height);
    printf("   FPS: %.1f\n", fps);
    printf("   Формат: %.0f (YUYV=1448695129)\n", fourcc);
    
    // Тест 4: Захват кадров
    printf("\n4️⃣ Тестирование захвата кадров...\n");
    cv::Mat frame;
    int success_count = 0;
    int total_attempts = 10;
    
    for (int i = 0; i < total_attempts; i++) {
        if (cap.read(frame)) {
            if (!frame.empty()) {
                success_count++;
                printf("   Кадр %d: ✅ %dx%d, %d каналов\n", 
                       i+1, frame.cols, frame.rows, frame.channels());
            } else {
                printf("   Кадр %d: ❌ Пустой\n", i+1);
            }
        } else {
            printf("   Кадр %d: ❌ Ошибка чтения\n", i+1);
        }
        usleep(100000); // 100 мс задержка
    }
    
    printf("\n📊 Результаты тестирования:\n");
    printf("   Успешных кадров: %d/%d (%.1f%%)\n", 
           success_count, total_attempts, 
           (double)success_count * 100 / total_attempts);
    
    if (success_count > 0) {
        printf("✅ USB DVR работает корректно!\n");
        printf("   Рекомендация: Программа готова к работе\n");
    } else {
        printf("❌ USB DVR не работает\n");
        printf("   Возможные причины:\n");
        printf("   - Нет сигнала на входе DVR\n");
        printf("   - Проблемы с драйверами\n");
        printf("   - Устройство занято другой программой\n");
    }
    
    cap.release();
    return 0;
}
