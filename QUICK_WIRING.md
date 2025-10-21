# 🔌 Быстрая схема подключения RX5808 к Raspberry Pi 4

## 📋 Краткая схема подключения

### **RX5808 → Raspberry Pi 4**

```
RX5808 Pin    →    Raspberry Pi 4 Pin    →    Функция
─────────────────────────────────────────────────────────
GND           →    Pin 6 (GND)          →    Земля
+5V           →    Pin 2 (5V)            →    Питание
RSSI          →    Pin 26 (GPIO 7)       →    Сила сигнала
VIDEO         →    USB Video DVR Input   →    Аналоговый видео
A6.5M         →    Pin 19 (GPIO 10)      →    SPI MOSI
CH1           →    Pin 23 (GPIO 11)      →    SPI SCK
CH2           →    Pin 24 (GPIO 8)       →    SPI CS
ANT           →    Антенна 5.8 ГГц       →    Антенна

USB Video DVR →    Raspberry Pi USB     →    Цифровой видео
```

**⚠️ ВАЖНО:**
- RX5808 выдает **аналоговый** видеосигнал
- USB Video DVR **оцифровывает** сигнал
- Raspberry Pi получает **цифровой** видеопоток

## 🎯 Быстрый старт

### **1. Подключение RX5808:**
```
RX5808 модуль:
┌─────────────────────────────────────┐
│ GND  ANT  GND                       │
│ GND  VIDEO  A  6.5M  RSSI  +5V  GND │
│ CH3  CH2   CH1                      │
└─────────────────────────────────────┘
```

### **2. Подключение к Raspberry Pi 4:**
```
Raspberry Pi 4 (вид сверху):
    3V3  (1) (2)  5V     ← RX5808 +5V
  GPIO2  (3) (4)  5V
  GPIO3  (5) (6)  GND    ← RX5808 GND
  GPIO4  (7) (8)  GPIO14
    GND  (9) (10) GPIO15
 GPIO17 (11) (12) GPIO18
 GPIO27 (13) (14) GND
 GPIO22 (15) (16) GPIO23
    3V3 (17) (18) GPIO24
 GPIO10 (19) (20) GND    ← RX5808 A6.5M (MOSI)
 GPIO11 (23) (24) GPIO8  ← RX5808 CH1 (SCK)
    GND (25) (26) GPIO7  ← RX5808 CH2 (CS)
                         ← RX5808 RSSI
```

### **3. USB Video DVR:**
- Подключите USB Video DVR к любому USB порту Raspberry Pi
- Подключите VIDEO выход RX5808 к видео входу USB DVR

### **4. Антенна:**
- Подключите антенну 5.8 ГГц к контакту ANT на RX5808
- Рекомендуется круговая поляризация для FPV

## ⚡ Быстрая проверка

### **Команды для проверки подключения:**

```bash
# Проверка SPI
ls /dev/spi*

# Проверка видео
ls /dev/video*

# Проверка GPIO
gpio readall

# Тест оборудования
python3 examples/test_hardware.py
```

## 🔧 Настройка системы

### **Включение SPI:**
```bash
sudo nano /boot/firmware/config.txt
# Добавить строку:
dtparam=spi=on

# Перезагрузка
sudo reboot
```

### **Установка зависимостей:**
```bash
sudo apt update
sudo apt install python3-rpi.gpio python3-spidev
sudo apt install python3-opencv python3-pil
pip3 install numpy pillow
```

## 🚀 Запуск

### **Быстрый запуск:**
```bash
# Простой сканер
python3 src/simple_scanner.py

# Продвинутый сканер
python3 src/advanced_scanner.py

# Меню выбора
python3 quick_start.py
```

## ⚠️ Важные замечания

### **Питание:**
- RX5808 может работать от 3.3V или 5V
- Для стабильной работы рекомендуется 5V
- Убедитесь в достаточной мощности блока питания

### **Заземление:**
- Обязательно подключите GND
- Используйте общую землю для всех компонентов

### **Антенна:**
- Используйте антенну на 5.8 ГГц
- Круговая поляризация лучше для FPV
- Расположите антенну для лучшего приема

### **USB Video DVR:**
- Убедитесь в совместимости с Linux
- Проверьте права доступа к устройству
- Тестируйте с `lsusb` и `ls /dev/video*`

## 🎯 Готово!

После подключения и настройки:
1. Запустите тест оборудования
2. Запустите простой сканер
3. Начните перехват FPV сигналов дронов!

**Удачного сканирования! 🚁📡**
