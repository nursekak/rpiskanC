# FPV Interceptor - Инструкция по использованию

## 🎯 Доступные версии

### 1. Базовая версия (без OpenCV)
```bash
make
./fpv_interceptor_gui
```
- ✅ RSSI анализ
- ✅ График в реальном времени
- ✅ Сканирование частот
- ❌ Видеозахват

### 2. Полная версия (с OpenCV)
```bash
# Установка OpenCV
./install_opencv.sh

# Сборка с OpenCV
make opencv
./fpv_interceptor_opencv
```
- ✅ RSSI анализ
- ✅ График в реальном времени
- ✅ Сканирование частот
- ✅ Видеозахват с USB Video DVR

## 🔧 Установка OpenCV

### Автоматическая установка
```bash
chmod +x install_opencv.sh
./install_opencv.sh
```

### Ручная установка
```bash
sudo apt update
sudo apt install -y libopencv-dev python3-opencv
```

## 📹 Настройка видеозахвата

### 1. Подключение оборудования
```
RX5808 VIDEO → USB Video DVR Input (аналоговый)
USB Video DVR → Raspberry Pi USB (цифровой)
```

### 2. Проверка подключения
```bash
# Проверка USB Video DVR
lsusb | grep -i video

# Проверка видеоустройства
ls -la /dev/video*
```

### 3. Запуск с видеозахватом
```bash
make opencv
./fpv_interceptor_opencv
```

## 🎛️ Использование GUI

### Основные функции
1. **🔍 Сканирование** - автоматический поиск FPV сигналов
2. **👁️ Мониторинг** - наблюдение за конкретной частотой
3. **📊 График RSSI** - визуализация в реальном времени
4. **📹 Видеозахват** - захват видео при обнаружении сигнала

### Интерфейс
- **Верхняя область**: Видео с USB Video DVR
- **Панель управления**: Кнопки и настройки
- **График RSSI**: Визуализация сигнала
- **Статус**: Информация о работе

## 🔍 Проверка перехвата видео

### Индикаторы успешного перехвата
1. **В GUI**: Видео отображается в верхней области
2. **В консоли**: Сообщения "📹 Захват видео..."
3. **Файлы**: Автоматическое сохранение в `captures/`

### Устранение проблем
```bash
# Проверка OpenCV
pkg-config --modversion opencv

# Проверка видеоустройства
ls -la /dev/video*

# Проверка USB Video DVR
lsusb | grep -i video

# Пересборка
make clean && make opencv
```

## 📊 Анализ результатов

### Файлы данных
- `captures/video_*.avi` - Захваченное видео
- `signals_*.txt` - Данные о сигналах
- `logs/` - Системные логи

### Параметры сигнала
- **Частота**: 5725-6000 МГц
- **RSSI**: 0-100%
- **Качество**: Автоматическая оценка
- **Движение**: Детекция движения

## 🚀 Быстрый старт

### Без видеозахвата
```bash
make
./fpv_interceptor_gui
```

### С видеозахватом
```bash
./install_opencv.sh
make opencv
./fpv_interceptor_opencv
```

## ⚠️ Важные замечания

1. **USB Video DVR обязателен** для видеозахвата
2. **Аналоговый сигнал** от RX5808 должен идти в USB Video DVR
3. **Цифровой сигнал** от USB Video DVR идет в Raspberry Pi
4. **OpenCV необходим** только для полного видеозахвата

## 🆘 Поддержка

### Полезные команды
```bash
# Проверка системы
make test-hardware

# Очистка
make clean

# Справка
make help
```

### Логи
```bash
# Системные логи
journalctl -u fpv-interceptor -f

# Проверка pigpio
sudo systemctl status pigpiod
```

---

**FPV Interceptor** - профессиональный перехват FPV сигналов с видеозахватом! 🚁📡📹
