# FPV Interceptor GUI Makefile для Raspberry Pi 4
# Система перехвата FPV видеосигналов 5.8 ГГц с GUI

# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -D_GNU_SOURCE
LDFLAGS = -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lpigpio -lpthread -lm

# GUI флаги
GUI_CFLAGS = $(CFLAGS) `pkg-config --cflags gtk+-3.0`
GUI_LDFLAGS = $(LDFLAGS) `pkg-config --libs gtk+-3.0`

# Имя исполняемого файла
TARGET = fpv_interceptor_gui

# Исходные файлы
SOURCES = fpv_gui_simple.c \
          rx5808_stub.c \
          rssi_analyzer.c \
          frequency_scanner_fixed.c

# Объектные файлы
OBJECTS = $(SOURCES:.c=.o)

# Заголовочные файлы
HEADERS = fpv_interceptor.h fpv_gui.h

# По умолчанию
all: $(TARGET)

# Сборка GUI версии
$(TARGET): $(OBJECTS)
	@echo "🔨 Сборка FPV Interceptor GUI..."
	$(CC) $(OBJECTS) -o $(TARGET) $(GUI_LDFLAGS)
	@echo "✅ GUI сборка завершена: $(TARGET)"

# Компиляция объектных файлов
%.o: %.c $(HEADERS)
	@echo "📦 Компиляция $<..."
	$(CC) $(GUI_CFLAGS) -c $< -o $@

# Очистка
clean:
	@echo "🧹 Очистка файлов сборки..."
	rm -f $(OBJECTS) $(TARGET)
	@echo "✅ Очистка завершена"

# Установка зависимостей
install-deps:
	@echo "📦 Установка зависимостей..."
	sudo apt update
	sudo apt install -y build-essential cmake pkg-config
	sudo apt install -y libopencv-dev python3-opencv
	sudo apt install -y libjpeg-dev libtiff5-dev libpng-dev
	sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev
	sudo apt install -y libgtk2.0-dev libcanberra-gtk*
	sudo apt install -y libv4l-dev v4l-utils
	sudo apt install -y python3-pip python3-dev
	sudo apt install -y libgtk-3-dev libcairo2-dev libpango1.0-dev
	sudo apt install -y git
	@echo "📦 Установка pigpio из исходников..."
	@if [ ! -d "pigpio" ]; then \
		git clone https://github.com/joan2937/pigpio.git; \
	fi
	cd pigpio && make && sudo make install
	@echo "✅ Зависимости установлены"

# Настройка системы
setup-system:
	@echo "⚙️ Настройка системы..."
	# Включение SPI
	echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
	echo "dtoverlay=spi0-2cs" | sudo tee -a /boot/firmware/config.txt
	# Включение I2C (для расширенных функций)
	echo "dtparam=i2c_arm=on" | sudo tee -a /boot/firmware/config.txt
	# Настройка GPIO
	echo "gpio=7=ip" | sudo tee -a /boot/firmware/config.txt
	# Настройка pigpio демона
	sudo systemctl enable pigpiod
	sudo systemctl start pigpiod
	@echo "✅ Система настроена, требуется перезагрузка"

# Проверка оборудования
test-hardware:
	@echo "🔧 Проверка оборудования..."
	@echo "Проверка SPI..."
	ls /dev/spi* 2>/dev/null || echo "❌ SPI не найден"
	@echo "Проверка pigpio..."
	sudo systemctl status pigpiod --no-pager -l || echo "❌ pigpio демон не запущен"
	@echo "Проверка видео..."
	ls /dev/video* 2>/dev/null || echo "❌ USB Video DVR не найден"
	@echo "Проверка USB Video DVR..."
	lsusb | grep -i "video\|dvr\|capture" || echo "❌ USB Video DVR не подключен"
	@echo "ℹ️ Убедитесь что USB Video DVR подключен к Raspberry Pi"
	@echo "ℹ️ И что аналоговый выход RX5808 подключен к входу USB Video DVR"
	@echo "Запуск Python теста..."
	python3 examples/test_hardware.py

# Создание директории для данных
create-dirs:
	@echo "📁 Создание директорий..."
	mkdir -p captures
	mkdir -p logs
	mkdir -p config
	@echo "✅ Директории созданы"

# Установка прав
set-permissions:
	@echo "🔐 Настройка прав доступа..."
	sudo usermod -a -G gpio $(USER)
	sudo usermod -a -G spi $(USER)
	sudo usermod -a -G video $(USER)
	@echo "✅ Права настроены"

# Создание systemd сервиса
install-service:
	@echo "🔧 Создание systemd сервиса..."
	@echo '[Unit]' > /tmp/fpv-interceptor.service
	@echo 'Description=FPV Interceptor GUI Service' >> /tmp/fpv-interceptor.service
	@echo 'After=network.target' >> /tmp/fpv-interceptor.service
	@echo '' >> /tmp/fpv-interceptor.service
	@echo '[Service]' >> /tmp/fpv-interceptor.service
	@echo 'Type=simple' >> /tmp/fpv-interceptor.service
	@echo 'User=pi' >> /tmp/fpv-interceptor.service
	@echo 'WorkingDirectory=/home/pi/fpv-interceptor' >> /tmp/fpv-interceptor.service
	@echo 'ExecStart=/home/pi/fpv-interceptor/fpv_interceptor_gui' >> /tmp/fpv-interceptor.service
	@echo 'Restart=always' >> /tmp/fpv-interceptor.service
	@echo 'RestartSec=5' >> /tmp/fpv-interceptor.service
	@echo '' >> /tmp/fpv-interceptor.service
	@echo '[Install]' >> /tmp/fpv-interceptor.service
	@echo 'WantedBy=multi-user.target' >> /tmp/fpv-interceptor.service
	@sudo cp /tmp/fpv-interceptor.service /etc/systemd/system/
	@sudo systemctl daemon-reload
	@sudo systemctl enable fpv-interceptor
	@echo "✅ Сервис установлен"

# Запуск сервиса
start-service:
	@echo "🚀 Запуск сервиса..."
	sudo systemctl start fpv-interceptor
	sudo systemctl status fpv-interceptor

# Остановка сервиса
stop-service:
	@echo "⏹️ Остановка сервиса..."
	sudo systemctl stop fpv-interceptor

# Создание скрипта быстрого запуска
create-launcher:
	@echo "🚀 Создание скрипта запуска..."
	@echo '#!/bin/bash' > fpv_gui.sh
	@echo '# FPV Interceptor GUI Launcher' >> fpv_gui.sh
	@echo '' >> fpv_gui.sh
	@echo 'echo "🚀 Запуск FPV Interceptor GUI..."' >> fpv_gui.sh
	@echo '' >> fpv_gui.sh
	@echo '# Проверка прав' >> fpv_gui.sh
	@echo 'if [ "$$EUID" -eq 0 ]; then' >> fpv_gui.sh
	@echo '    echo "❌ Не запускайте от имени root!"' >> fpv_gui.sh
	@echo '    exit 1' >> fpv_gui.sh
	@echo 'fi' >> fpv_gui.sh
	@echo '' >> fpv_gui.sh
	@echo '# Проверка оборудования' >> fpv_gui.sh
	@echo 'echo "🔧 Проверка оборудования..."' >> fpv_gui.sh
	@echo 'if [ ! -e /dev/spi0.0 ]; then' >> fpv_gui.sh
	@echo '    echo "❌ SPI не найден. Проверьте настройки."' >> fpv_gui.sh
	@echo '    exit 1' >> fpv_gui.sh
	@echo 'fi' >> fpv_gui.sh
	@echo '' >> fpv_gui.sh
	@echo 'if [ ! -e /dev/video0 ]; then' >> fpv_gui.sh
	@echo '    echo "⚠️ Видеоустройство не найдено. Видеозахват недоступен."' >> fpv_gui.sh
	@echo 'fi' >> fpv_gui.sh
	@echo '' >> fpv_gui.sh
	@echo '# Запуск программы' >> fpv_gui.sh
	@echo 'echo "✅ Запуск FPV Interceptor GUI..."' >> fpv_gui.sh
	@echo './fpv_interceptor_gui' >> fpv_gui.sh
	@chmod +x fpv_gui.sh
	@echo "✅ Скрипт запуска создан"

# Создание конфигурационного файла
create-config:
	@echo "⚙️ Создание конфигурации..."
	@echo '# FPV Interceptor GUI Configuration' > config/fpv_config.conf
	@echo '' >> config/fpv_config.conf
	@echo '# Диапазон частот (МГц)' >> config/fpv_config.conf
	@echo 'FREQ_MIN=5725' >> config/fpv_config.conf
	@echo 'FREQ_MAX=6000' >> config/fpv_config.conf
	@echo 'FREQ_STEP=1' >> config/fpv_config.conf
	@echo '' >> config/fpv_config.conf
	@echo '# RSSI настройки' >> config/fpv_config.conf
	@echo 'RSSI_THRESHOLD=50' >> config/fpv_config.conf
	@echo 'RSSI_SAMPLES=100' >> config/fpv_config.conf
	@echo '' >> config/fpv_config.conf
	@echo '# Видео настройки' >> config/fpv_config.conf
	@echo 'VIDEO_WIDTH=640' >> config/fpv_config.conf
	@echo 'VIDEO_HEIGHT=480' >> config/fpv_config.conf
	@echo 'VIDEO_FPS=30' >> config/fpv_config.conf
	@echo '' >> config/fpv_config.conf
	@echo '# Сканирование' >> config/fpv_config.conf
	@echo 'SCAN_DWELL_TIME=100' >> config/fpv_config.conf
	@echo 'SCAN_TIMEOUT=5000' >> config/fpv_config.conf
	@echo '' >> config/fpv_config.conf
	@echo '# GPIO пины' >> config/fpv_config.conf
	@echo 'CS_PIN=8' >> config/fpv_config.conf
	@echo 'MOSI_PIN=10' >> config/fpv_config.conf
	@echo 'MISO_PIN=9' >> config/fpv_config.conf
	@echo 'SCK_PIN=11' >> config/fpv_config.conf
	@echo 'RSSI_PIN=7' >> config/fpv_config.conf
	@echo '' >> config/fpv_config.conf
	@echo '# Видеоустройство' >> config/fpv_config.conf
	@echo 'VIDEO_DEVICE=/dev/video0' >> config/fpv_config.conf
	@echo "✅ Конфигурация создана"

# Полная установка
install: install-deps setup-system create-dirs set-permissions create-config create-launcher
	@echo "🎯 Полная установка завершена!"
	@echo "Перезагрузите систему: sudo reboot"
	@echo "Затем запустите: ./fpv_gui.sh"

# Отладка
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)
	@echo "🐛 Сборка отладочной версии завершена"

# Профилирование
profile: CFLAGS += -pg
profile: LDFLAGS += -pg
profile: $(TARGET)
	@echo "📊 Сборка с профилированием завершена"

# Статическая сборка
static: LDFLAGS += -static
static: $(TARGET)
	@echo "📦 Статическая сборка завершена"

# Создание архива
package: clean
	@echo "📦 Создание архива..."
	tar -czf fpv_interceptor_gui.tar.gz *.c *.h Makefile README.md
	@echo "✅ Архив создан: fpv_interceptor_gui.tar.gz"

# Помощь
help:
	@echo "FPV Interceptor GUI - Система перехвата FPV сигналов с GUI"
	@echo ""
	@echo "Доступные команды:"
	@echo "  make              - Сборка GUI программы"
	@echo "  make clean         - Очистка файлов сборки"
	@echo "  make install-deps - Установка зависимостей"
	@echo "  make setup-system - Настройка системы"
	@echo "  make test-hardware- Проверка оборудования"
	@echo "  make install      - Полная установка"
	@echo "  make debug        - Отладочная сборка"
	@echo "  make profile      - Сборка с профилированием"
	@echo "  make static       - Статическая сборка"
	@echo "  make package      - Создание архива"
	@echo "  make help         - Показать эту справку"

# Зависимости
$(OBJECTS): $(HEADERS)

# Файлы, которые не являются реальными файлами
.PHONY: all clean install-deps setup-system test-hardware create-dirs set-permissions install-service start-service stop-service create-launcher create-config install debug profile static package help

# Информация о сборке
info:
	@echo "📊 Информация о сборке:"
	@echo "  Компилятор: $(CC)"
	@echo "  Флаги: $(CFLAGS)"
	@echo "  Библиотеки: $(LDFLAGS)"
	@echo "  Цель: $(TARGET)"
	@echo "  Исходники: $(SOURCES)"