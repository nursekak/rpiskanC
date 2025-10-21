#ifndef FPV_GUI_H
#define FPV_GUI_H

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo/cairo.h>
#include "fpv_interceptor.h"

// Константы GUI
#define GUI_WIDTH           1200
#define GUI_HEIGHT          800
#define VIDEO_WIDTH         800
#define VIDEO_HEIGHT        400
#define CHART_HEIGHT        150
#define RSSI_HISTORY_SIZE   100

// Функции GUI
GtkWidget* create_main_window(void);
void update_status(const char *message);
void update_rssi_display(uint8_t rssi, uint16_t frequency);
gboolean draw_rssi_chart(GtkWidget *widget, cairo_t *cr, gpointer data);
gboolean update_video_display(gpointer data);
gboolean scan_frequencies(gpointer data);

// Обработчики событий
void on_scan_button_clicked(GtkWidget *widget, gpointer data);
void on_stop_button_clicked(GtkWidget *widget, gpointer data);
void on_monitor_frequency(GtkWidget *widget, gpointer data);
gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);

#endif // FPV_GUI_H
