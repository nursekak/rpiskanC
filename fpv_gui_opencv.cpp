#include "fpv_interceptor.h"
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo/cairo.h>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>

// Глобальные переменные GUI
static GtkWidget *video_widget;
static GtkWidget *status_label;
static GtkWidget *rssi_progress;
static GtkWidget *frequency_label;
static GtkWidget *scan_button;
static GtkWidget *stop_button;
static GtkWidget *frequency_entry;
static GtkWidget *rssi_chart;

// Данные для графика RSSI
static double rssi_history[100];
static int rssi_index = 0;
static int rssi_count = 0;

// Состояние программы
static gboolean scanning = FALSE;
static gboolean video_capturing = FALSE;
static guint scan_timer_id = 0;
static guint video_timer_id = 0;

// OpenCV переменные
static cv::VideoCapture *video_capture = nullptr;
static cv::Mat current_frame;
static pthread_mutex_t frame_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Конвертация OpenCV Mat в GdkPixbuf
 */
GdkPixbuf* convert_mat_to_pixbuf(const cv::Mat &mat) {
    if (mat.empty()) return nullptr;
    
    cv::Mat rgb_mat;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgb_mat, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 1) {
        cv::cvtColor(mat, rgb_mat, cv::COLOR_GRAY2RGB);
    } else {
        return nullptr;
    }
    
    // Создание GdkPixbuf
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(
        rgb_mat.data,
        GDK_COLORSPACE_RGB,
        FALSE,
        8,
        rgb_mat.cols,
        rgb_mat.rows,
        rgb_mat.step,
        nullptr,
        nullptr
    );
    
    return pixbuf;
}

/**
 * Инициализация видеозахвата
 */
extern "C" int init_video_capture(void) {
    printf("📹 Инициализация видеозахвата с OpenCV...\n");
    
    // Проверка доступности видеоустройства
    if (access("/dev/video0", F_OK) != 0) {
        printf("❌ USB Video DVR не найден: /dev/video0\n");
        printf("ℹ️ Подключите USB Video DVR к Raspberry Pi\n");
        printf("ℹ️ Подключите аналоговый выход RX5808 к входу USB Video DVR\n");
        return -1;
    }
    
    // Создание объекта захвата видео
    video_capture = new cv::VideoCapture();
    
    // Открытие видеоустройства
    if (!video_capture->open("/dev/video0")) {
        printf("❌ Ошибка открытия видеоустройства: /dev/video0\n");
        delete video_capture;
        video_capture = nullptr;
        return -1;
    }
    
    // Настройка параметров видео
    video_capture->set(cv::CAP_PROP_FRAME_WIDTH, 640);
    video_capture->set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    video_capture->set(cv::CAP_PROP_FPS, 30);
    video_capture->set(cv::CAP_PROP_BUFFERSIZE, 1);
    
    printf("✅ Видеозахват инициализирован с OpenCV\n");
    return 0;
}

/**
 * Обновление статуса в GUI
 */
extern "C" void update_status(const char *message) {
    gtk_label_set_text(GTK_LABEL(status_label), message);
    gtk_widget_queue_draw(status_label);
}

/**
 * Обновление RSSI в GUI
 */
extern "C" void update_rssi_display(uint8_t rssi, uint16_t frequency) {
    // Обновление прогресс-бара RSSI
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(rssi_progress), rssi / 100.0);
    
    // Обновление метки частоты
    char freq_text[64];
    snprintf(freq_text, sizeof(freq_text), "Частота: %d МГц | RSSI: %d%%", frequency, rssi);
    gtk_label_set_text(GTK_LABEL(frequency_label), freq_text);
    
    // Добавление в историю для графика
    rssi_history[rssi_index] = rssi;
    rssi_index = (rssi_index + 1) % 100;
    if (rssi_count < 100) rssi_count++;
    
    // Перерисовка графика
    gtk_widget_queue_draw(rssi_chart);
}

/**
 * Отрисовка графика RSSI
 */
gboolean draw_rssi_chart(GtkWidget *widget, cairo_t *cr, gpointer data) {
    (void)data; // Подавление предупреждения
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    
    int width = allocation.width;
    int height = allocation.height;
    
    // Очистка фона
    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
    
    if (rssi_count < 2) return FALSE;
    
    // Настройка линий
    cairo_set_line_width(cr, 2.0);
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); // Зеленый цвет
    
    // Построение графика
    double x_step = (double)width / (rssi_count - 1);
    double y_scale = (double)height / 100.0;
    
    cairo_move_to(cr, 0, height - rssi_history[0] * y_scale);
    
    for (int i = 1; i < rssi_count; i++) {
        double x = i * x_step;
        double y = height - rssi_history[i] * y_scale;
        cairo_line_to(cr, x, y);
    }
    
    cairo_stroke(cr);
    
    return FALSE;
}

/**
 * Обновление видео в GUI
 */
gboolean update_video_display(gpointer data) {
    (void)data; // Подавление предупреждения
    if (!video_capturing || !video_capture) return FALSE;
    
    cv::Mat frame;
    
    // Захват кадра
    if (video_capture->read(frame) && !frame.empty()) {
        // Блокировка мьютекса для обновления кадра
        pthread_mutex_lock(&frame_mutex);
        current_frame = frame.clone();
        pthread_mutex_unlock(&frame_mutex);
        
        // Конвертация OpenCV Mat в GdkPixbuf
        GdkPixbuf *pixbuf = convert_mat_to_pixbuf(frame);
        if (pixbuf) {
            // Обновление виджета видео
            gtk_image_set_from_pixbuf(GTK_IMAGE(video_widget), pixbuf);
            g_object_unref(pixbuf);
        }
        
        update_status("📹 Захват видео...");
    } else {
        update_status("⚠️ Нет видеосигнала");
    }
    
    return TRUE;
}

/**
 * Сканирование частот в GUI
 */
gboolean scan_frequencies(gpointer data) {
    (void)data; // Подавление предупреждения
    if (!scanning) return FALSE;
    
    static uint16_t current_freq = FREQ_MIN;
    
    // Установка частоты
    if (rx5808_set_frequency(current_freq) == 0) {
        // Чтение RSSI
        uint8_t rssi = rx5808_read_rssi();
        
        // Обновление GUI
        update_rssi_display(rssi, current_freq);
        
        // Проверка на обнаружение сигнала
        if (rssi > RSSI_THRESHOLD) {
            char message[256];
            snprintf(message, sizeof(message), 
                    "🎯 Сигнал обнаружен: %d МГц, RSSI: %d%%", current_freq, rssi);
            update_status(message);
            
            // Начало захвата видео
            if (!video_capturing && video_capture) {
                video_capturing = TRUE;
                video_timer_id = g_timeout_add(100, update_video_display, NULL);
            }
        }
        
        // Переход к следующей частоте
        current_freq += FREQ_STEP;
        if (current_freq > FREQ_MAX) {
            current_freq = FREQ_MIN;
        }
    }
    
    return TRUE;
}

/**
 * Обработчик кнопки "Начать сканирование"
 */
void on_scan_button_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data; // Подавление предупреждений
    if (!scanning) {
        scanning = TRUE;
        gtk_button_set_label(GTK_BUTTON(scan_button), "⏹️ Остановить");
        gtk_widget_set_sensitive(stop_button, TRUE);
        
        update_status("🔍 Начало сканирования...");
        
        // Запуск таймера сканирования
        scan_timer_id = g_timeout_add(100, scan_frequencies, NULL);
    } else {
        scanning = FALSE;
        gtk_button_set_label(GTK_BUTTON(scan_button), "🔍 Начать сканирование");
        gtk_widget_set_sensitive(stop_button, FALSE);
        
        update_status("⏹️ Сканирование остановлено");
        
        // Остановка таймеров
        if (scan_timer_id) {
            g_source_remove(scan_timer_id);
            scan_timer_id = 0;
        }
        if (video_timer_id) {
            g_source_remove(video_timer_id);
            video_timer_id = 0;
            video_capturing = FALSE;
        }
    }
}

/**
 * Обработчик кнопки "Остановить"
 */
void on_stop_button_clicked(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data; // Подавление предупреждений
    scanning = FALSE;
    video_capturing = FALSE;
    
    gtk_button_set_label(GTK_BUTTON(scan_button), "🔍 Начать сканирование");
    gtk_widget_set_sensitive(stop_button, FALSE);
    
    update_status("⏹️ Все операции остановлены");
    
    // Остановка таймеров
    if (scan_timer_id) {
        g_source_remove(scan_timer_id);
        scan_timer_id = 0;
    }
    if (video_timer_id) {
        g_source_remove(video_timer_id);
        video_timer_id = 0;
    }
}

/**
 * Обработчик кнопки "Мониторинг частоты"
 */
void on_monitor_frequency(GtkWidget *widget, gpointer data) {
    (void)widget; (void)data; // Подавление предупреждений
    const char *freq_text = gtk_entry_get_text(GTK_ENTRY(frequency_entry));
    int frequency = atoi(freq_text);
    
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) {
        update_status("❌ Неверная частота. Диапазон: 5725-6000 МГц");
        return;
    }
    
    update_status("👁️ Мониторинг частоты...");
    
    // Установка частоты
    if (rx5808_set_frequency(frequency) == 0) {
        uint8_t rssi = rx5808_read_rssi();
        update_rssi_display(rssi, frequency);
        update_status("✅ Частота установлена");
    } else {
        update_status("❌ Ошибка установки частоты");
    }
}

/**
 * Создание главного окна
 */
GtkWidget* create_main_window(void) {
    GtkWidget *window;
    GtkWidget *vbox, *hbox;
    GtkWidget *video_frame, *control_frame;
    GtkWidget *separator;
    
    // Главное окно
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "FPV Interceptor - Перехват FPV сигналов 5.8 ГГц");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    
    // Основной контейнер
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    // Область видео
    video_frame = gtk_frame_new("📹 Видео (USB Video DVR) - OpenCV");
    gtk_frame_set_shadow_type(GTK_FRAME(video_frame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(vbox), video_frame, TRUE, TRUE, 5);
    
    video_widget = gtk_image_new();
    gtk_widget_set_size_request(video_widget, 800, 400);
    gtk_container_add(GTK_CONTAINER(video_frame), video_widget);
    
    // Разделитель
    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);
    
    // Панель управления
    control_frame = gtk_frame_new("🎛️ Управление");
    gtk_frame_set_shadow_type(GTK_FRAME(control_frame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(vbox), control_frame, FALSE, FALSE, 5);
    
    GtkWidget *control_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(control_frame), control_vbox);
    
    // Строка с кнопками
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(control_vbox), hbox, FALSE, FALSE, 5);
    
    scan_button = gtk_button_new_with_label("🔍 Начать сканирование");
    gtk_widget_set_size_request(scan_button, 200, 40);
    gtk_box_pack_start(GTK_BOX(hbox), scan_button, FALSE, FALSE, 5);
    g_signal_connect(scan_button, "clicked", G_CALLBACK(on_scan_button_clicked), NULL);
    
    stop_button = gtk_button_new_with_label("⏹️ Остановить");
    gtk_widget_set_size_request(stop_button, 150, 40);
    gtk_widget_set_sensitive(stop_button, FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), stop_button, FALSE, FALSE, 5);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop_button_clicked), NULL);
    
    // Поле ввода частоты
    GtkWidget *freq_label = gtk_label_new("Частота (МГц):");
    gtk_box_pack_start(GTK_BOX(hbox), freq_label, FALSE, FALSE, 5);
    
    frequency_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(frequency_entry), "5800");
    gtk_widget_set_size_request(frequency_entry, 100, 30);
    gtk_box_pack_start(GTK_BOX(hbox), frequency_entry, FALSE, FALSE, 5);
    
    GtkWidget *monitor_button = gtk_button_new_with_label("👁️ Мониторинг");
    gtk_widget_set_size_request(monitor_button, 120, 30);
    gtk_box_pack_start(GTK_BOX(hbox), monitor_button, FALSE, FALSE, 5);
    g_signal_connect(monitor_button, "clicked", G_CALLBACK(on_monitor_frequency), NULL);
    
    // Строка с индикаторами
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(control_vbox), hbox, FALSE, FALSE, 5);
    
    // Индикатор RSSI
    GtkWidget *rssi_label = gtk_label_new("RSSI:");
    gtk_box_pack_start(GTK_BOX(hbox), rssi_label, FALSE, FALSE, 5);
    
    rssi_progress = gtk_progress_bar_new();
    gtk_widget_set_size_request(rssi_progress, 200, 25);
    gtk_box_pack_start(GTK_BOX(hbox), rssi_progress, FALSE, FALSE, 5);
    
    // Метка частоты и RSSI
    frequency_label = gtk_label_new("Частота: -- МГц | RSSI: --%");
    gtk_box_pack_start(GTK_BOX(hbox), frequency_label, FALSE, FALSE, 5);
    
    // График RSSI
    GtkWidget *chart_frame = gtk_frame_new("📊 График RSSI");
    gtk_frame_set_shadow_type(GTK_FRAME(chart_frame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(control_vbox), chart_frame, FALSE, FALSE, 5);
    
    rssi_chart = gtk_drawing_area_new();
    gtk_widget_set_size_request(rssi_chart, 800, 150);
    gtk_container_add(GTK_CONTAINER(chart_frame), rssi_chart);
    g_signal_connect(rssi_chart, "draw", G_CALLBACK(draw_rssi_chart), NULL);
    
    // Статус
    status_label = gtk_label_new("🚀 FPV Interceptor готов к работе (с OpenCV)");
    gtk_label_set_xalign(GTK_LABEL(status_label), 0.0);
    gtk_box_pack_start(GTK_BOX(control_vbox), status_label, FALSE, FALSE, 5);
    
    return window;
}

/**
 * Обработчик закрытия окна
 */
gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
    (void)widget; (void)event; (void)data; // Подавление предупреждений
    // Остановка всех операций
    scanning = FALSE;
    video_capturing = FALSE;
    
    if (scan_timer_id) {
        g_source_remove(scan_timer_id);
    }
    if (video_timer_id) {
        g_source_remove(video_timer_id);
    }
    
    // Очистка OpenCV
    if (video_capture) {
        video_capture->release();
        delete video_capture;
        video_capture = nullptr;
    }
    
    gtk_main_quit();
    return FALSE;
}

/**
 * Главная функция GUI
 */
extern "C" int main(int argc, char *argv[]) {
    GtkWidget *window;
    
    // Инициализация GTK
    gtk_init(&argc, &argv);
    
    printf("🚀 Запуск FPV Interceptor GUI с OpenCV...\n");
    
    // Инициализация модулей
    if (rx5808_init() != 0) {
        printf("❌ Ошибка инициализации RX5808\n");
        return -1;
    }
    
    if (rssi_analyzer_init() != 0) {
        printf("❌ Ошибка инициализации анализатора RSSI\n");
        return -1;
    }
    
    if (init_video_capture() != 0) {
        printf("⚠️ Предупреждение: USB Video DVR недоступен\n");
        printf("ℹ️ Видеозахват будет недоступен, но RSSI анализ работает\n");
    }
    
    if (frequency_scanner_init() != 0) {
        printf("❌ Ошибка инициализации сканера частот\n");
        return -1;
    }
    
    // Создание главного окна
    window = create_main_window();
    
    // Обработчики событий
    g_signal_connect(window, "delete-event", G_CALLBACK(on_window_delete_event), NULL);
    
    // Отображение окна
    gtk_widget_show_all(window);
    
    printf("✅ GUI запущен с OpenCV\n");
    
    // Запуск главного цикла GTK
    gtk_main();
    
    return 0;
}
