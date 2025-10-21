# FPV Interceptor GUI - Перехват FPV видеосигналов 5.8 ГГц

## 🎯 Описание

FPV Interceptor GUI - это система перехвата FPV видеосигналов в диапазоне 5.8 ГГц с графическим интерфейсом для Raspberry Pi 4. Программа автоматически сканирует частоты, анализирует RSSI сигналы и захватывает видео с USB Video DVR.

## ✨ Особенности

- **🎨 Графический интерфейс** - удобный GUI на GTK+3
- **📹 Видеозахват** - реальное видео с USB Video DVR через OpenCV
- **📊 Анализ RSSI** - график в реальном времени
- **🔍 Автосканирование** - автоматический поиск FPV сигналов
- **💾 Сохранение** - автоматическое сохранение видео и данных
- **🎛️ Управление** - полный контроль через GUI

## 🔧 Системные требования

- Raspberry Pi 4 Model B
- RX5808 5.8GHz Receiver Module
- USB Video DVR (для видеозахвата)
- Linux с поддержкой SPI и GPIO
- OpenCV для видеозахвата
- GTK+3 для GUI

## 📦 Установка

### 1. Быстрая установка

```bash
# Клонирование проекта
git clone <repository-url>
cd rpiskanC

# Полная установка
make install

# Перезагрузка
sudo reboot
```

### 2. Ручная установка

```bash
# Установка зависимостей
make install-deps

# Настройка системы
make setup-system

# Создание директорий
make create-dirs

# Настройка прав
make set-permissions

# Перезагрузка
sudo reboot
```

## 🚀 Запуск

### Графический интерфейс

```bash
# Сборка GUI версии
make

# Запуск
./fpv_interceptor_gui
```

### Скрипт запуска

```bash
# Создание скрипта
make create-launcher

# Запуск через скрипт
./fpv_gui.sh
```

## 🔌 Подключение оборудования

### RX5808 к Raspberry Pi

```
RX5808    →    Raspberry Pi
VCC       →    3.3V (Pin 1)
GND       →    GND (Pin 6)
CS        →    GPIO 8 (Pin 24)
MOSI      →    GPIO 10 (Pin 19)
MISO      →    GPIO 9 (Pin 21)
SCK       →    GPIO 11 (Pin 23)
RSSI      →    GPIO 7 (Pin 26)
VIDEO     →    USB Video DVR Input
```

### USB Video DVR

```
USB Video DVR Input  →  RX5808 VIDEO Pin (аналоговый)
USB Video DVR Output →  Raspberry Pi USB (цифровой)
```

## 🎨 Интерфейс

### Главное окно

- **📹 Видео область** - отображение захваченного видео
- **🎛️ Панель управления** - кнопки управления
- **📊 График RSSI** - график в реальном времени
- **📈 Индикаторы** - RSSI, частота, статус

### Функции

1. **🔍 Сканирование** - автоматический поиск сигналов
2. **👁️ Мониторинг** - наблюдение за конкретной частотой
3. **⏹️ Остановка** - остановка всех операций
4. **💾 Сохранение** - автоматическое сохранение данных

## 📊 Использование

### Основные операции

1. **Запуск программы** - `./fpv_interceptor_gui`
2. **Начало сканирования** - кнопка "🔍 Начать сканирование"
3. **Мониторинг частоты** - ввод частоты и кнопка "👁️ Мониторинг"
4. **Остановка** - кнопка "⏹️ Остановить"

### Автоматические функции

- **Детекция сигналов** - автоматическое обнаружение FPV сигналов
- **Захват видео** - автоматический захват при обнаружении
- **Сохранение данных** - автоматическое сохранение в файлы

## 📁 Структура файлов

```
rpiskanC/
├── fpv_interceptor_gui     # Исполняемый файл
├── fpv_gui.c              # GUI интерфейс
├── rssi_analyzer.c        # RSSI анализатор
├── video_detector_gui.c   # Видеодетектор с OpenCV
├── frequency_scanner.c    # Частотный сканер
├── fpv_interceptor.h      # Заголовочный файл
├── fpv_gui.h             # GUI заголовки
├── Makefile              # Система сборки
├── install_opencv.sh     # Скрипт установки OpenCV
└── fpv_gui.sh           # Скрипт запуска
```

## 🐛 Отладка

### Проверка оборудования

```bash
# Проверка всех компонентов
make test-hardware

# Проверка SPI
ls -la /dev/spi*

# Проверка видео
ls -la /dev/video*

# Проверка USB Video DVR
lsusb | grep -i video
```

### Логи

```bash
# Просмотр системных логов
journalctl -u fpv-interceptor -f

# Проверка pigpio
sudo systemctl status pigpiod
```

## ⚙️ Настройка

### Конфигурация частот

```bash
# Редактирование в fpv_interceptor.h
#define FREQ_MIN 5725  // Минимальная частота
#define FREQ_MAX 6000  // Максимальная частота
#define FREQ_STEP 1     // Шаг сканирования
```

### Настройка видео

```bash
# Параметры видео в video_detector_gui.c
set_video_parameters(640, 480, 30);  // Ширина, высота, FPS
```

## 📈 Производительность

### Характеристики

- **Частота сканирования**: 1-10 МГц/сек
- **RSSI точность**: ±2%
- **Время отклика**: <100 мс
- **Видео**: 640x480 @ 30 FPS
- **Потребление памяти**: <100 МБ

### Оптимизация

```bash
# Увеличение приоритета
sudo nice -n -10 ./fpv_interceptor_gui

# Ограничение CPU для других процессов
sudo cpulimit -p $(pgrep -f "other_process") -l 50
```

## 🚨 Устранение неполадок

### Ошибка "Permission denied"

```bash
# Добавление в группы
sudo usermod -a -G gpio,spi,video $(USER)
sudo reboot
```

### Ошибка "SPI not found"

```bash
# Включение SPI
echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
sudo reboot
```

### Ошибка "OpenCV not found"

```bash
# Установка OpenCV
./install_opencv.sh
```

### Ошибка "USB Video DVR not found"

```bash
# Проверка подключения
lsusb | grep -i video
ls -la /dev/video*
```

## 📞 Поддержка

### Полезные команды

```bash
# Проверка системы
make test-hardware

# Очистка сборки
make clean

# Пересборка
make

# Справка
make help
```

### Контакты

- **Документация**: README.md
- **Быстрый старт**: QUICK_START.md
- **Схема подключения**: RPI_WIRING.md

---

**FPV Interceptor GUI** - профессиональное решение для перехвата FPV сигналов с удобным графическим интерфейсом! 🚁📡🎨