# 📦 Установка FPV Interceptor

## 🎯 Системные требования

### Оборудование:
- **Raspberry Pi 4 Model B** (рекомендуется 4GB RAM)
- **RX5808 5.8GHz Receiver Module**
- **USB Video DVR** (совместимый с Linux)
- **5.8GHz Антенна** (круговая поляризация)
- **Макетная плата** и соединительные провода
- **Блок питания** (5V, минимум 3A)

### Программное обеспечение:
- **Raspberry Pi OS** (Bullseye или новее)
- **Python 3.7+**
- **OpenCV 4.0+**
- **WiringPi**

## 🔧 Пошаговая установка

### Шаг 1: Подготовка системы

```bash
# Обновление системы
sudo apt update && sudo apt upgrade -y

# Установка необходимых пакетов
sudo apt install -y git build-essential cmake pkg-config
sudo apt install -y python3-pip python3-dev python3-venv
sudo apt install -y libopencv-dev python3-opencv
sudo apt install -y wiringpi libwiringpi-dev
sudo apt install -y libjpeg-dev libtiff5-dev libpng-dev
sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev
sudo apt install -y libgtk2.0-dev libcanberra-gtk*
sudo apt install -y libv4l-dev v4l-utils
```

### Шаг 2: Настройка GPIO и SPI

```bash
# Включение SPI
echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
echo "dtoverlay=spi0-2cs" | sudo tee -a /boot/firmware/config.txt

# Включение I2C (для расширенных функций)
echo "dtparam=i2c_arm=on" | sudo tee -a /boot/firmware/config.txt

# Настройка GPIO пинов
echo "gpio=7=ip" | sudo tee -a /boot/firmware/config.txt

# Перезагрузка для применения изменений
sudo reboot
```

### Шаг 3: Клонирование проекта

```bash
# Создание рабочей директории
mkdir -p ~/fpv-interceptor
cd ~/fpv-interceptor

# Клонирование репозитория (замените на ваш URL)
git clone <repository-url> .

# Или скачивание архива
wget <archive-url>
tar -xzf fpv_interceptor.tar.gz
```

### Шаг 4: Сборка программы

```bash
# Установка зависимостей
make install-deps

# Сборка программы
make

# Создание необходимых директорий
make create-dirs

# Настройка прав доступа
make set-permissions
```

### Шаг 5: Проверка оборудования

```bash
# Проверка SPI
ls /dev/spi*

# Проверка GPIO
gpio readall

# Проверка видеоустройств
ls /dev/video*

# Запуск теста оборудования
python3 examples/test_hardware.py
```

### Шаг 6: Первый запуск

```bash
# Запуск программы
./fpv_interceptor.sh

# Или прямой запуск
./fpv_interceptor
```

## 🔌 Подключение оборудования

### Схема подключения RX5808:

```
RX5808 Pin    →    Raspberry Pi 4 Pin    →    Функция
─────────────────────────────────────────────────────────
GND           →    Pin 6 (GND)          →    Земля
+5V           →    Pin 2 (5V)            →    Питание
RSSI          →    Pin 26 (GPIO 7)       →    Сила сигнала
VIDEO         →    USB Video DVR         →    Видео выход
A6.5M         →    Pin 19 (GPIO 10)      →    SPI MOSI
CH1           →    Pin 23 (GPIO 11)      →    SPI SCK
CH2           →    Pin 24 (GPIO 8)       →    SPI CS
ANT           →    Антенна 5.8 ГГц       →    Антенна
```

### Подключение USB Video DVR:

1. Подключите USB Video DVR к любому USB порту Raspberry Pi
2. Подключите VIDEO выход RX5808 к видео входу USB DVR
3. Проверьте подключение: `lsusb` и `ls /dev/video*`

### Подключение антенны:

1. Используйте антенну на 5.8 ГГц
2. Рекомендуется круговая поляризация для FPV
3. Подключите к контакту ANT на RX5808

## ⚙️ Конфигурация

### Основные настройки:

```bash
# Редактирование конфигурации
nano config/fpv_config.conf
```

### Настройка частот:

```ini
# Диапазон частот (МГц)
FREQ_MIN=5725
FREQ_MAX=6000
FREQ_STEP=1
```

### Настройка RSSI:

```ini
# RSSI настройки
RSSI_THRESHOLD=50
RSSI_SAMPLES=100
```

### Настройка видео:

```ini
# Видео настройки
VIDEO_WIDTH=640
VIDEO_HEIGHT=480
VIDEO_FPS=30
```

## 🚀 Автоматический запуск

### Создание systemd сервиса:

```bash
# Установка сервиса
make install-service

# Запуск сервиса
make start-service

# Проверка статуса
sudo systemctl status fpv-interceptor
```

### Настройка автозапуска:

```bash
# Включение автозапуска
sudo systemctl enable fpv-interceptor

# Отключение автозапуска
sudo systemctl disable fpv-interceptor
```

## 🔧 Устранение неполадок

### Проблема: SPI не работает

```bash
# Проверка SPI
ls /dev/spi*

# Если не найдено, проверьте настройки
cat /boot/firmware/config.txt | grep spi

# Перезагрузка
sudo reboot
```

### Проблема: GPIO недоступен

```bash
# Проверка прав пользователя
groups $USER

# Добавление пользователя в группу gpio
sudo usermod -a -G gpio $USER

# Перезагрузка
sudo reboot
```

### Проблема: Видео не захватывается

```bash
# Проверка видеоустройств
ls /dev/video*

# Проверка USB устройств
lsusb

# Проверка прав доступа
sudo chmod 666 /dev/video*
```

### Проблема: Слабый сигнал

1. Проверьте подключение антенны
2. Убедитесь в правильной поляризации
3. Проверьте заземление
4. Попробуйте другую антенну

### Проблема: Ошибки компиляции

```bash
# Обновление зависимостей
sudo apt update
sudo apt install -y build-essential cmake pkg-config

# Переустановка OpenCV
sudo apt remove -y libopencv-dev
sudo apt install -y libopencv-dev

# Очистка и пересборка
make clean
make
```

## 📊 Тестирование

### Тест оборудования:

```bash
# Полный тест оборудования
python3 examples/test_hardware.py

# Простой тест сканера
python3 examples/simple_scanner.py
```

### Тест производительности:

```bash
# Сборка отладочной версии
make debug

# Запуск с профилированием
make profile
```

## 🔒 Безопасность

### Настройка прав доступа:

```bash
# Настройка прав для GPIO
sudo usermod -a -G gpio $USER

# Настройка прав для SPI
sudo usermod -a -G spi $USER

# Настройка прав для видео
sudo usermod -a -G video $USER
```

### Ограничение доступа:

```bash
# Создание отдельного пользователя
sudo useradd -m -s /bin/bash fpv
sudo usermod -a -G gpio,spi,video fpv

# Запуск от имени пользователя fpv
sudo -u fpv ./fpv_interceptor
```

## 📈 Оптимизация

### Настройка производительности:

```bash
# Увеличение GPU памяти
echo "gpu_mem=128" | sudo tee -a /boot/firmware/config.txt

# Отключение ненужных сервисов
sudo systemctl disable bluetooth
sudo systemctl disable hciuart
```

### Настройка сети:

```bash
# Статический IP (опционально)
sudo nano /etc/dhcpcd.conf
```

## 📝 Логи и мониторинг

### Просмотр логов:

```bash
# Логи systemd сервиса
journalctl -u fpv-interceptor -f

# Логи системы
dmesg | grep -i spi
dmesg | grep -i gpio
```

### Мониторинг ресурсов:

```bash
# Мониторинг CPU и памяти
htop

# Мониторинг температуры
vcgencmd measure_temp

# Мониторинг напряжения
vcgencmd measure_volts
```

## 🆘 Поддержка

### Получение помощи:

1. Проверьте файл README.md
2. Изучите примеры в директории examples/
3. Создайте issue в репозитории
4. Обратитесь к документации

### Отладка:

```bash
# Сборка отладочной версии
make debug

# Запуск с отладкой
gdb ./fpv_interceptor

# Анализ core dump
gdb ./fpv_interceptor core
```

---

**🎯 FPV Interceptor готов к работе!**

После успешной установки вы можете:
- Запустить полное сканирование диапазона
- Настроить непрерывный мониторинг
- Анализировать обнаруженные сигналы
- Захватывать видеопотоки

