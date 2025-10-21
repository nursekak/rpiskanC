#include "fpv_interceptor.h"

// Глобальные переменные
channel_data_t channels[CHANNELS_COUNT];
detected_signal_t detected_signals[MAX_SIGNALS];
uint8_t signal_count = 0;
volatile int running = 1;

/**
 * Обработчик сигналов для корректного завершения
 * @param sig Номер сигнала
 */
void signal_handler(int sig) {
    printf("\n🛑 Получен сигнал %d, завершение работы...\n", sig);
    running = 0;
}

/**
 * Добавление обнаруженного сигнала в список
 * @param frequency Частота сигнала
 * @param rssi Уровень сигнала
 * @param video Наличие видеосигнала
 * @return 0 при успехе, -1 при ошибке
 */
int add_detected_signal(uint16_t frequency, uint8_t rssi, uint8_t video) {
    if (signal_count >= MAX_SIGNALS) {
        printf("⚠️ Достигнуто максимальное количество сигналов\n");
        return -1;
    }
    
    // Проверка на дубликаты
    for (int i = 0; i < signal_count; i++) {
        if (detected_signals[i].frequency == frequency) {
            // Обновление существующего сигнала
            detected_signals[i].rssi = rssi;
            detected_signals[i].video_detected = video;
            detected_signals[i].timestamp = get_timestamp();
            return 0;
        }
    }
    
    // Добавление нового сигнала
    detected_signals[signal_count].frequency = frequency;
    detected_signals[signal_count].rssi = rssi;
    detected_signals[signal_count].video_detected = video;
    detected_signals[signal_count].timestamp = get_timestamp();
    
    // Создание имени файла для видео
    if (video) {
        snprintf(detected_signals[signal_count].video_file, 
                sizeof(detected_signals[signal_count].video_file),
                "video_%d_%u.avi", frequency, get_timestamp());
    }
    
    signal_count++;
    printf("✅ Сигнал добавлен: %d МГц, RSSI: %d%%, Видео: %s\n", 
           frequency, rssi, video ? "Да" : "Нет");
    
    return 0;
}

/**
 * Вывод списка обнаруженных сигналов
 */
void print_detected_signals(void) {
    if (signal_count == 0) {
        printf("📭 Обнаруженные сигналы не найдены\n");
        return;
    }
    
    printf("\n📋 Обнаруженные сигналы (%d):\n", signal_count);
    printf("┌─────────────┬─────────┬─────────┬─────────────────────┬─────────────────┐\n");
    printf("│ Частота     │ RSSI    │ Видео   │ Время обнаружения   │ Видеофайл        │\n");
    printf("├─────────────┼─────────┼─────────┼─────────────────────┼─────────────────┤\n");
    
    for (int i = 0; i < signal_count; i++) {
        printf("│ %-11d │ %-7d │ %-7s │ %-19u │ %-15s │\n",
               detected_signals[i].frequency,
               detected_signals[i].rssi,
               detected_signals[i].video_detected ? "Да" : "Нет",
               detected_signals[i].timestamp,
               detected_signals[i].video_detected ? 
               detected_signals[i].video_file : "Нет");
    }
    
    printf("└─────────────┴─────────┴─────────┴─────────────────────┴─────────────────┘\n");
}

/**
 * Сохранение данных сигналов в файл
 */
void save_signal_data(void) {
    char filename[256];
    snprintf(filename, sizeof(filename), "signals_%u.txt", get_timestamp());
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("❌ Ошибка создания файла: %s\n", filename);
        return;
    }
    
    fprintf(file, "# FPV Interceptor - Обнаруженные сигналы\n");
    fprintf(file, "# Время сканирования: %u\n", get_timestamp());
    fprintf(file, "# Диапазон: %d-%d МГц\n", FREQ_MIN, FREQ_MAX);
    fprintf(file, "# Обнаружено сигналов: %d\n\n", signal_count);
    
    for (int i = 0; i < signal_count; i++) {
        fprintf(file, "Сигнал %d:\n", i + 1);
        fprintf(file, "  Частота: %d МГц\n", detected_signals[i].frequency);
        fprintf(file, "  RSSI: %d%%\n", detected_signals[i].rssi);
        fprintf(file, "  Видеосигнал: %s\n", detected_signals[i].video_detected ? "Да" : "Нет");
        fprintf(file, "  Время: %u\n", detected_signals[i].timestamp);
        if (detected_signals[i].video_detected) {
            fprintf(file, "  Видеофайл: %s\n", detected_signals[i].video_file);
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    printf("💾 Данные сигналов сохранены: %s\n", filename);
}

/**
 * Вывод статуса сканирования
 * @param frequency Текущая частота
 * @param rssi Уровень RSSI
 */
void print_status(uint16_t frequency, uint8_t rssi) {
    static uint32_t last_status_time = 0;
    uint32_t current_time = get_timestamp();
    
    // Вывод статуса каждые 5 секунд
    if (current_time - last_status_time > 5000) {
        printf("📡 Статус: %d МГц, RSSI: %d%%\n", frequency, rssi);
        last_status_time = current_time;
    }
}

/**
 * Получение текущего времени в миллисекундах
 * @return Время в миллисекундах
 */
uint32_t get_timestamp(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/**
 * Очистка ресурсов
 */
void cleanup_resources(void) {
    printf("🧹 Очистка ресурсов...\n");
    
    // Остановка сканирования
    if (running) {
        running = 0;
    }
    
    // Очистка модулей
    frequency_scanner_cleanup();
    rssi_analyzer_cleanup();
    video_detector_cleanup();
    rx5808_cleanup_linux();
    
    printf("✅ Очистка завершена\n");
}

/**
 * Отображение главного меню
 */
void show_menu(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    FPV Interceptor v1.0                      ║\n");
    printf("║              Перехват FPV сигналов 5.8 ГГц                  ║\n");
    printf("║                    Raspberry Pi 4 Model B                    ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ 1. Полное сканирование диапазона (%d-%d МГц)                ║\n", FREQ_MIN, FREQ_MAX);
    printf("║ 2. Непрерывное сканирование                                 ║\n");
    printf("║ 3. Сканирование диапазона                                  ║\n");
    printf("║ 4. Мониторинг частоты                                       ║\n");
    printf("║ 5. Показать обнаруженные сигналы                           ║\n");
    printf("║ 6. Сохранить данные                                         ║\n");
    printf("║ 7. Информация о системе                                     ║\n");
    printf("║ 8. Тест оборудования                                        ║\n");
    printf("║ 0. Выход                                                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("Выберите опцию: ");
}

/**
 * Тест оборудования
 */
void test_hardware(void) {
    printf("🔧 Тестирование оборудования...\n");
    
    // Тест RX5808
    printf("📡 Тест модуля RX5808...\n");
    rx5808_get_info();
    
    // Тест RSSI
    printf("📊 Тест анализатора RSSI...\n");
    uint8_t rssi = rx5808_read_rssi();
    printf("   Текущий RSSI: %d%%\n", rssi);
    
    // Тест видео
    printf("📹 Тест видеозахвата...\n");
    get_video_info();
    
    // Тест сканера
    printf("🔍 Тест сканера частот...\n");
    get_scan_stats();
    
    printf("✅ Тест оборудования завершен\n");
}

/**
 * Информация о системе
 */
void show_system_info(void) {
    printf("📊 Информация о системе:\n");
    printf("   Платформа: Raspberry Pi 4 Model B\n");
    printf("   Диапазон частот: %d-%d МГц\n", FREQ_MIN, FREQ_MAX);
    printf("   Шаг сканирования: %d МГц\n", FREQ_STEP);
    printf("   Порог RSSI: %d%%\n", RSSI_THRESHOLD);
    printf("   Разрешение видео: %dx%d\n", VIDEO_WIDTH, VIDEO_HEIGHT);
    printf("   FPS видео: %d\n", VIDEO_FPS);
    printf("   Максимум сигналов: %d\n", MAX_SIGNALS);
    printf("   Образцы RSSI: %d\n", RSSI_SAMPLES);
}

/**
 * Основная функция
 */
int main(int argc, char *argv[]) {
    (void)argc;  // Подавление предупреждения о неиспользуемом параметре
    (void)argv;  // Подавление предупреждения о неиспользуемом параметре
    printf("🚀 Запуск FPV Interceptor...\n");
    
    // Установка обработчиков сигналов
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Инициализация модулей
    printf("🔧 Инициализация модулей...\n");
    
    if (rx5808_init_linux() != 0) {
        printf("❌ Ошибка инициализации RX5808\n");
        return -1;
    }
    
    if (rssi_analyzer_init() != 0) {
        printf("❌ Ошибка инициализации анализатора RSSI\n");
        cleanup_resources();
        return -1;
    }
    
    if (init_video_capture() != 0) {
        printf("⚠️ Предупреждение: USB Video DVR недоступен\n");
        printf("ℹ️ Подключите USB Video DVR для оцифровки аналогового сигнала RX5808\n");
    }
    
    if (frequency_scanner_init() != 0) {
        printf("❌ Ошибка инициализации сканера частот\n");
        cleanup_resources();
        return -1;
    }
    
    printf("✅ Все модули инициализированы\n");
    
    // Главный цикл программы
    int choice;
    while (running) {
        show_menu();
        
        if (scanf("%d", &choice) != 1) {
            printf("❌ Неверный ввод\n");
            continue;
        }
        
        switch (choice) {
            case 1:
                printf("🔍 Запуск полного сканирования...\n");
                scan_frequency_range();
                break;
                
            case 2:
                printf("🔄 Запуск непрерывного сканирования...\n");
                start_continuous_scan();
                printf("Нажмите Enter для остановки...\n");
                getchar();
                getchar();
                stop_continuous_scan();
                break;
                
            case 3: {
                uint16_t start_freq, end_freq;
                printf("Введите начальную частоту (МГц): ");
                scanf("%hu", &start_freq);
                printf("Введите конечную частоту (МГц): ");
                scanf("%hu", &end_freq);
                scan_frequency_range_custom(start_freq, end_freq);
                break;
            }
            
            case 4: {
                uint16_t frequency;
                uint32_t duration;
                printf("Введите частоту для мониторинга (МГц): ");
                scanf("%hu", &frequency);
                printf("Введите время мониторинга в секундах (0 = бесконечно): ");
                scanf("%u", &duration);
                monitor_frequency(frequency, duration);
                break;
            }
            
            case 5:
                print_detected_signals();
                break;
                
            case 6:
                save_signal_data();
                break;
                
            case 7:
                show_system_info();
                break;
                
            case 8:
                test_hardware();
                break;
                
            case 0:
                printf("👋 Завершение работы...\n");
                running = 0;
                break;
                
            default:
                printf("❌ Неверный выбор\n");
                break;
        }
    }
    
    // Очистка ресурсов
    cleanup_resources();
    
    printf("✅ Программа завершена\n");
    return 0;
}

