# 🔧 Установка без pigpio (Linux GPIO)

## 📋 Проблема с pigpio

Если пакет `pigpio` недоступен в репозиториях, используйте **Linux GPIO версию** без внешних зависимостей.

## 🚀 Быстрая установка

### **1. Установка базовых зависимостей:**
```bash
# Установка без pigpio
sudo apt update
sudo apt install -y build-essential cmake pkg-config
sudo apt install -y libopencv-dev python3-opencv
sudo apt install -y libjpeg-dev libtiff5-dev libpng-dev
sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev
sudo apt install -y libgtk2.0-dev libcanberra-gtk*
sudo apt install -y libv4l-dev v4l-utils
sudo apt install -y python3-pip python3-dev
sudo apt install -y libgtk-3-dev libcairo2-dev libpango1.0-dev
```

### **2. Настройка системы:**
```bash
# Включение SPI
echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
echo "dtoverlay=spi0-2cs" | sudo tee -a /boot/firmware/config.txt

# Перезагрузка
sudo reboot
```

### **3. Сборка Linux GPIO версии:**
```bash
# Сборка без pigpio
make linux

# Или GUI версия
make gui
```

## 🔧 Как работает Linux GPIO версия

### **Вместо pigpio использует:**
- **`/sys/class/gpio/`** - стандартный Linux GPIO интерфейс
- **`/dev/spidev0.0`** - стандартный Linux SPI интерфейс
- **`ioctl()`** - системные вызовы для SPI

### **Преимущества:**
- ✅ **Нет внешних зависимостей** - только стандартные библиотеки
- ✅ **Работает на любом Linux** - не требует специальных пакетов
- ✅ **Стабильная работа** - использует системные интерфейсы
- ✅ **Простая установка** - не нужно компилировать pigpio

## 📁 Файлы Linux GPIO версии

### **Основные файлы:**
- **`gpio_linux.c`** - реализация GPIO через sysfs
- **`fpv_interceptor_linux`** - исполняемый файл
- **`fpv_gui`** - GUI версия (если нужна)

### **Функции:**
```c
// Инициализация
int rx5808_init_linux(void);

// Установка частоты
int rx5808_set_frequency_linux(uint16_t frequency);

// Чтение RSSI
uint8_t rx5808_read_rssi_linux(void);

// Очистка
int rx5808_cleanup_linux(void);
```

## 🚀 Запуск

### **Консольная версия:**
```bash
# Запуск Linux GPIO версии
./fpv_interceptor_linux
```

### **GUI версия:**
```bash
# Запуск GUI версии
./fpv_interceptor_gui
```

## 🔧 Настройка GPIO

### **Права доступа:**
```bash
# Добавление пользователя в группу gpio
sudo usermod -a -G gpio $USER

# Перезагрузка для применения
sudo reboot
```

### **Проверка GPIO:**
```bash
# Проверка доступности GPIO
ls /sys/class/gpio/

# Экспорт пина (пример)
echo 7 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio7/direction
cat /sys/class/gpio/gpio7/value
```

## 📊 Сравнение версий

| Функция | pigpio версия | Linux GPIO версия |
|---------|---------------|-------------------|
| **Зависимости** | pigpio библиотека | Только стандартные |
| **Установка** | Сложная | Простая |
| **Производительность** | Высокая | Средняя |
| **Стабильность** | Высокая | Высокая |
| **Совместимость** | Raspberry Pi | Любой Linux |

## 🛠️ Устранение проблем

### **GPIO недоступен:**
```bash
# Проверка прав
groups $USER

# Добавление в группу gpio
sudo usermod -a -G gpio $USER
sudo reboot
```

### **SPI не работает:**
```bash
# Проверка SPI
ls /dev/spi*

# Включение SPI
echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
sudo reboot
```

### **Ошибки компиляции:**
```bash
# Очистка и пересборка
make clean
make linux
```

## 🎯 Рекомендации

### **Используйте Linux GPIO версию если:**
- ❌ pigpio недоступен в репозиториях
- ❌ Проблемы с компиляцией pigpio
- ✅ Нужна простая установка
- ✅ Работа на разных дистрибутивах Linux

### **Используйте pigpio версию если:**
- ✅ pigpio доступен и работает
- ✅ Нужна максимальная производительность
- ✅ Продвинутые функции GPIO

## 📝 Примеры использования

### **Тест GPIO:**
```bash
# Экспорт пина
echo 7 > /sys/class/gpio/export

# Настройка как вход
echo in > /sys/class/gpio/gpio7/direction

# Чтение значения
cat /sys/class/gpio/gpio7/value

# Очистка
echo 7 > /sys/class/gpio/unexport
```

### **Тест SPI:**
```bash
# Проверка SPI устройств
ls /dev/spi*

# Тест SPI (требует root)
sudo dd if=/dev/zero of=/dev/spidev0.0 bs=1 count=1
```

---

**🎯 Linux GPIO версия готова к работе!**

Теперь у вас есть рабочая версия FPV Interceptor без зависимости от pigpio, использующая стандартные Linux интерфейсы.
