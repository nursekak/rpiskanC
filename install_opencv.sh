#!/bin/bash
# Скрипт установки OpenCV для FPV Interceptor GUI

echo "🚀 Установка OpenCV для FPV Interceptor GUI..."

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
sudo apt install -y libgtk-3-dev libcairo2-dev libpango1.0-dev

# Проверка установки OpenCV
echo "🔍 Проверка установки OpenCV..."
if pkg-config --exists opencv4; then
    echo "✅ OpenCV 4 найден"
    pkg-config --modversion opencv4
elif pkg-config --exists opencv; then
    echo "✅ OpenCV найден"
    pkg-config --modversion opencv
else
    echo "❌ OpenCV не найден, попытка установки из исходников..."
    
    # Установка из исходников
    cd /tmp
    git clone https://github.com/opencv/opencv.git
    git clone https://github.com/opencv/opencv_contrib.git
    
    cd opencv
    mkdir build
    cd build
    
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
          -D CMAKE_INSTALL_PREFIX=/usr/local \
          -D OPENCV_EXTRA_MODULES_PATH=/tmp/opencv_contrib/modules \
          -D EIGEN_INCLUDE_PATH=/usr/include/eigen3 \
          -D BUILD_EXAMPLES=OFF \
          -D INSTALL_PYTHON_EXAMPLES=OFF \
          -D INSTALL_C_EXAMPLES=OFF \
          -D OPENCV_ENABLE_NONFREE=ON \
          -D CMAKE_INSTALL_PREFIX=/usr/local \
          -D BUILD_TESTS=OFF \
          -D BUILD_PERF_TESTS=OFF \
          -D BUILD_opencv_python2=OFF \
          -D BUILD_opencv_python3=ON \
          -D OPENCV_GENERATE_PKGCONFIG=ON \
          -D CMAKE_BUILD_TYPE=RELEASE \
          -D CMAKE_INSTALL_PREFIX=/usr/local \
          -D OPENCV_EXTRA_MODULES_PATH=/tmp/opencv_contrib/modules \
          -D EIGEN_INCLUDE_PATH=/usr/include/eigen3 \
          -D BUILD_EXAMPLES=OFF \
          -D INSTALL_PYTHON_EXAMPLES=OFF \
          -D INSTALL_C_EXAMPLES=OFF \
          -D OPENCV_ENABLE_NONFREE=ON \
          -D CMAKE_INSTALL_PREFIX=/usr/local \
          -D BUILD_TESTS=OFF \
          -D BUILD_PERF_TESTS=OFF \
          -D BUILD_opencv_python2=OFF \
          -D BUILD_opencv_python3=ON \
          -D OPENCV_GENERATE_PKGCONFIG=ON \
          ..
    
    make -j$(nproc)
    sudo make install
    sudo ldconfig
fi

# Проверка видеоустройств
echo "📹 Проверка видеоустройств..."
ls -la /dev/video* 2>/dev/null || echo "⚠️ Видеоустройства не найдены"

# Проверка USB Video DVR
echo "🔍 Проверка USB Video DVR..."
lsusb | grep -i "video\|dvr\|capture" || echo "⚠️ USB Video DVR не подключен"

echo "✅ Установка OpenCV завершена!"
echo "ℹ️ Для использования GUI версии:"
echo "   make gui"
echo "   ./fpv_interceptor_gui"
