#include "fpv_interceptor.h"
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo/cairo.h>

// Глобальные переменные GUI
static GtkWidget *main_window;
static GtkWidget *video_widget;
static GtkWidget *status_label;
static GtkWidget *rssi_progress;
static GtkWidget *frequency_label;
static GtkWidget *scan_button;
static GtkWidget *stop_button;
static GtkWidget *frequency_entry;
static GtkWidget *rssi_chart;
static GtkWidget *log_text;

// Данные для графика RSSI
static double rssi_history[100];
static int rssi_index = 0;
static int rssi_count = 0;

// Состояние программы
static gboolean scanning = FALSE;
static gboolean video_capturing = FALSE;
static guint scan_timer_id = 0;
static guint video_timer_id = 0;

/**
 * Обновление статуса в GUI
 */
void update_status(const char *message) {
    gtk_label_set_text(GTK_LABEL(status_label), message);
    gtk_widget_queue_draw(status_label);
}

/**
 * Обновление RSSI в GUI
 */
void update_rssi_display(uint8_t rssi, uint16_t frequency) {
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
    
    // Добавление сетки
    cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 0.5);
    cairo_set_line_width(cr, 1.0);
    
    for (int i = 0; i <= 10; i++) {
        double y = (height / 10.0) * i;
        cairo_move_to(cr, 0, y);
        cairo_line_to(cr, width, y);
        cairo_stroke(cr);
    }
    
    return FALSE;
}

/**
 * Обновление видео в GUI (заглушка)
 */
gboolean update_video_display(gpointer data) {
    if (!video_capturing) return FALSE;
    
    // Заглушка для видео - показываем статус
    update_status("📹 Захват видео... (OpenCV не установлен)");
    
    return TRUE;
}

/**
 * Сканирование частот в GUI (заглушка)
 */
gboolean scan_frequencies(gpointer data) {
    if (!scanning) return FALSE;
    
    static uint16_t current_freq = FREQ_MIN;
    
    // Заглушка для сканирования - симулируем RSSI
    uint8_t rssi = 30 + (rand() % 40); // Случайный RSSI 30-70
    
    // Обновление GUI
    update_rssi_display(rssi, current_freq);
    
    // Проверка на обнаружение сигнала
    if (rssi > RSSI_THRESHOLD) {
        char message[256];
        snprintf(message, sizeof(message), 
                "🎯 Сигнал обнаружен: %d МГц, RSSI: %d%%", current_freq, rssi);
        update_status(message);
        
        // Начало захвата видео
        if (!video_capturing) {
            video_capturing = TRUE;
            video_timer_id = g_timeout_add(100, update_video_display, NULL);
        }
    }
    
    // Переход к следующей частоте
    current_freq += FREQ_STEP;
    if (current_freq > FREQ_MAX) {
        current_freq = FREQ_MIN;
    }
    
    return TRUE;
}

/**
 * Обработчик кнопки "Начать сканирование"
 */
void on_scan_button_clicked(GtkWidget *widget, gpointer data) {
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
    const char *freq_text = gtk_entry_get_text(GTK_ENTRY(frequency_entry));
    int frequency = atoi(freq_text);
    
    if (frequency < FREQ_MIN || frequency > FREQ_MAX) {
        update_status("❌ Неверная частота. Диапазон: 5725-6000 МГц");
        return;
    }
    
    update_status("👁️ Мониторинг частоты...");
    
    // Заглушка для мониторинга
    uint8_t rssi = 40 + (rand() % 30); // Случайный RSSI 40-70
    update_rssi_display(rssi, frequency);
    update_status("✅ Частота установлена (заглушка)");
}

/**
 * Создание главного окна
 */
GtkWidget* create_main_window(void) {
    GtkWidget *window;
    GtkWidget *vbox, *hbox;
    GtkWidget *video_frame, *control_frame;
    GtkWidget *button_box;
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
    video_frame = gtk_frame_new("📹 Видео (USB Video DVR) - OpenCV не установлен");
    gtk_frame_set_shadow_type(GTK_FRAME(video_frame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(vbox), video_frame, TRUE, TRUE, 5);
    
    video_widget = gtk_label_new("📹 Видеозахват недоступен\nУстановите OpenCV для полного функционала");
    gtk_label_set_justify(GTK_LABEL(video_widget), GTK_JUSTIFY_CENTER);
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
    status_label = gtk_label_new("🚀 FPV Interceptor готов к работе (упрощенный режим)");
    gtk_label_set_xalign(GTK_LABEL(status_label), 0.0);
    gtk_box_pack_start(GTK_BOX(control_vbox), status_label, FALSE, FALSE, 5);
    
    return window;
}

/**
 * Обработчик закрытия окна
 */
gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
    // Остановка всех операций
    scanning = FALSE;
    video_capturing = FALSE;
    
    if (scan_timer_id) {
        g_source_remove(scan_timer_id);
    }
    if (video_timer_id) {
        g_source_remove(video_timer_id);
    }
    
    gtk_main_quit();
    return FALSE;
}

/**
 * Главная функция GUI
 */
int main(int argc, char *argv[]) {
    GtkWidget *window;
    
    // Инициализация GTK
    gtk_init(&argc, &argv);
    
    printf("🚀 Запуск FPV Interceptor GUI (упрощенный режим)...\n");
    printf("ℹ️ OpenCV не установлен - используется упрощенный режим\n");
    
    // Создание главного окна
    window = create_main_window();
    
    // Обработчики событий
    g_signal_connect(window, "delete-event", G_CALLBACK(on_window_delete_event), NULL);
    
    // Отображение окна
    gtk_widget_show_all(window);
    
    printf("✅ GUI запущен (упрощенный режим)\n");
    
    // Запуск главного цикла GTK
    gtk_main();
    
    return 0;
}
