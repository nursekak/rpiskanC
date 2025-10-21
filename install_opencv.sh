#!/bin/bash
# Скрипт установки OpenCV для FPV Interceptor

echo "🚀 Установка OpenCV для FPV Interceptor..."

# Обновление системы
echo "📦 Обновление системы..."
sudo apt update
sudo apt upgrade -y

# Установка зависимостей
echo "📦 Установка зависимостей..."
sudo apt install -y build-essential cmake pkg-config
sudo apt install -y libopencv-dev python3-opencv
sudo apt install -y libjpeg-dev libtiff5-dev libpng-dev
sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev
sudo apt install -y libgtk2.0-dev libcanberra-gtk*
sudo apt install -y libv4l-dev v4l-utils

# Проверка установки OpenCV
echo "🔍 Проверка установки OpenCV..."
if pkg-config --exists opencv4; then
    echo "✅ OpenCV 4 найден"
    pkg-config --modversion opencv4
elif pkg-config --exists opencv; then
    echo "✅ OpenCV найден"
    pkg-config --modversion opencv
else
    echo "❌ OpenCV не найден"
    exit 1
fi

# Проверка видеоустройств
echo "📹 Проверка видеоустройств..."
ls -la /dev/video* 2>/dev/null || echo "⚠️ Видеоустройства не найдены"

# Проверка USB Video DVR
echo "🔍 Проверка USB Video DVR..."
lsusb | grep -i "video\|dvr\|capture" || echo "⚠️ USB Video DVR не подключен"

echo "✅ Установка OpenCV завершена!"
echo "ℹ️ Для использования видеозахвата:"
echo "   1. Подключите USB Video DVR к Raspberry Pi"
echo "   2. Подключите аналоговый выход RX5808 к входу USB Video DVR"
echo "   3. Пересоберите программу: make clean && make"
echo "   4. Запустите: ./fpv_interceptor_gui"