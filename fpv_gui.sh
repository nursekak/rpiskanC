#!/bin/bash
# FPV Interceptor GUI Launcher

echo "🚀 Запуск FPV Interceptor GUI..."

# Проверка прав
if [ "$EUID" -eq 0 ]; then
    echo "❌ Не запускайте от имени root!"
    exit 1
fi

# Проверка оборудования
echo "🔧 Проверка оборудования..."
if [ ! -e /dev/spi0.0 ]; then
    echo "❌ SPI не найден. Проверьте настройки."
    exit 1
fi

# Проверка pigpio
if ! systemctl is-active --quiet pigpiod; then
    echo "⚠️ pigpio демон не запущен. Запускаем..."
    sudo systemctl start pigpiod
fi

if [ ! -e /dev/video0 ]; then
    echo "⚠️ USB Video DVR не найден. Видеозахват недоступен."
fi

# Проверка GUI зависимостей
if ! pkg-config --exists gtk+-3.0; then
    echo "❌ GTK+3 не найден. Установите: sudo apt install libgtk-3-dev"
    exit 1
fi

# Запуск GUI
echo "✅ Запуск FPV Interceptor GUI..."
./fpv_interceptor_gui
