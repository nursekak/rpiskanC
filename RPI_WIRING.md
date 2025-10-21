# 🔌 Raspberry Pi 4 + RX5808 5.8GHz Scanner Wiring Guide

## 📋 Требования к оборудованию

### **Основные компоненты:**
- **Raspberry Pi 4 Model B** (рекомендуется 4GB RAM)
- **RX5808 5.8GHz Receiver Module** (модуль приемника)
- **USB Video DVR** (для захвата видео)
- **5.8GHz Антенна** (рекомендуется круговая поляризация)
- **Макетная плата** и соединительные провода
- **Блок питания** (5V, минимум 3A)

### **Дополнительные компоненты:**
- **ADC модуль** (MCP3008) для лучшего чтения RSSI
- **Видео усилитель** для усиления сигнала
- **Радиатор** для модуля RX5808
- **Корпус** для защиты

## 🔌 Подключение контактов

### **Распиновка RX5808:**
```
Распиновка модуля RX5808:
┌─────────────────────────────────────┐
│ GND  ANT  GND                       │
│ GND  VIDEO  A  6.5M  RSSI  +5V  GND │
│ CH3  CH2   CH1                      │
└─────────────────────────────────────┘
```

### **RX5808 к Raspberry Pi 4:**

```
RX5808 Pin           Функция           Raspberry Pi 4
─────────────────    ──────────        ──────────────────
GND                  Земля             GND (Pin 6)
+5V                  Питание           5V (Pin 2) или 3.3V (Pin 1)
RSSI                 Сила сигнала     GPIO 7 (Pin 26)
VIDEO                Видео выход       Вход USB Video DVR
A6.5M                SPI MOSI          GPIO 10 (Pin 19)
CH1                  SPI SCK           GPIO 11 (Pin 23)
CH2                  SPI CS            GPIO 8 (Pin 24)
CH3                  Не используется   -
ANT                  Антенна           Разъем антенны
```

### **USB Video DVR:**
```
USB Video DVR          Raspberry Pi 4
─────────────────      ──────────────────
USB Connector    ────── USB Port (any)
Video Input      ────── RX5808 VIDEO Pin (аналоговый)
Audio Input      ────── (Not available on RX5808)

⚠️ ВАЖНО: RX5808 → Аналоговый видео → USB Video DVR → Цифровой видео → Raspberry Pi
```

## 📍 Detailed Pin Mapping

### **SPI Interface (RX5808 Communication):**
```
Raspberry Pi 4 Pin    Function          RX5808 Pin
─────────────────     ──────────        ──────────
Pin 24 (GPIO 8)      CS (Chip Select)   CH2
Pin 19 (GPIO 10)     MOSI               A
Pin 21 (GPIO 9)      MISO               6.5M
Pin 23 (GPIO 11)     SCK                CH1
```

### **Power Supply:**
```
Raspberry Pi 4 Pin    Function          RX5808 Pin
─────────────────     ──────────        ──────────
Pin 2 (5V)           VCC                +5V
Pin 6 (GND)          Ground             GND
```

### **RSSI Reading:**
```
Raspberry Pi 4 Pin    Function          RX5808 Pin
─────────────────     ──────────        ──────────
Pin 26 (GPIO 7)      RSSI Input         RSSI
```

### **Video Output:**
```
RX5808 Pin            Function          Destination
─────────────────     ──────────        ──────────
VIDEO                 Video Output      USB Video DVR
```

## 🔧 Hardware Setup

### **Step 1: Enable SPI on Raspberry Pi**
```bash
# Edit config file
sudo nano /boot/firmware/config.txt

# Add these lines:
dtparam=spi=on
dtoverlay=spi0-2cs

# Reboot
sudo reboot
```

### **Step 2: Install Required Packages**
```bash
# Update system
sudo apt update && sudo apt upgrade

# Install Python packages
sudo apt install python3-pip python3-dev
sudo apt install python3-opencv python3-pil python3-pil.imagetk

# Install GPIO and SPI libraries
sudo apt install python3-rpi.gpio python3-spidev

# Install additional packages
pip3 install numpy pillow
```

### **Step 3: Physical Connections**

#### **RX5808 Module Connections:**
1. **Power Connections:**
   - Connect RX5808 +5V to Pi 5V (Pin 2) or 3.3V (Pin 1)
   - Connect RX5808 GND to Pi GND (Pin 6)

2. **SPI Connections:**
   - Connect RX5808 CH2 (CS) to Pi GPIO 8 (Pin 24)
   - Connect RX5808 A (MOSI) to Pi GPIO 10 (Pin 19)
   - Connect RX5808 6.5M (MISO) to Pi GPIO 9 (Pin 21)
   - Connect RX5808 CH1 (SCK) to Pi GPIO 11 (Pin 23)

3. **RSSI Connection:**
   - Connect RX5808 RSSI to Pi GPIO 7 (Pin 26)

4. **Video Connection:**
   - Connect RX5808 VIDEO to USB Video DVR input

#### **Antenna Connection:**
- Connect 5.8GHz antenna to RX5808 ANT pin
- Use SMA or RP-SMA connector as required

#### **USB Video DVR:**
- Insert USB Video DVR into any USB port
- Connect RX5808 VIDEO output to DVR video input

## 🎯 Alternative RSSI Reading Methods

### **Method 1: Direct GPIO Reading (Basic)**
```
RX5808 RSSI ──── GPIO 7 (Pin 26)
```
- Simple but limited resolution
- Good for basic signal detection

### **Method 2: ADC Module (Recommended)**
```
RX5808 RSSI ──── MCP3008 Channel 0
MCP3008      ──── Raspberry Pi SPI
```
- Better resolution (10-bit vs 1-bit)
- More accurate signal strength measurement

### **Method 3: External ADC (High Precision)**
```
RX5808 RSSI ──── ADS1115 A0
ADS1115      ──── Raspberry Pi I2C
```
- 16-bit resolution
- Best for precise measurements

## 📊 Connection Diagram

```
                    ┌─────────────────┐
                    │  Raspberry Pi 4 │
                    │                 │
                    │  ┌─────────────┐ │
                    │  │    USB      │ │
                    │  │ Video DVR   │ │
                    │  └─────────────┘ │
                    │                 │
                    │  ┌─────────────┐ │
                    │  │    SPI      │ │
                    │  │ Interface   │ │
                    │  └─────────────┘ │
                    └─────────────────┘
                              │
                    ┌─────────▼─────────┐
                    │    RX5808         │
                    │  5.8GHz Module    │
                    │                   │
                    │  ┌─────────────┐  │
                    │  │     ANT     │  │
                    │  │  (Antenna)  │  │
                    │  └─────────────┘  │
                    │                   │
                    │  ┌─────────────┐  │
                    │  │    VIDEO    │  │
                    │  │   Output    │  │
                    │  └─────────────┘  │
                    └───────────────────┘
```

## 🔍 Testing Connections

### **Test SPI Communication:**
```python
import spidev
import RPi.GPIO as GPIO

# Setup
GPIO.setmode(GPIO.BCM)
GPIO.setup(8, GPIO.OUT)  # CS pin (CH2)

spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 2000000

# Test write
GPIO.output(8, GPIO.LOW)
spi.writebytes([0x01, 0x02])  # Test data
GPIO.output(8, GPIO.HIGH)

print("SPI test completed")
```

### **Test RSSI Reading:**
```python
import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setup(7, GPIO.IN)  # RSSI pin

for i in range(10):
    rssi = GPIO.input(7)
    print(f"RSSI: {rssi}")
    time.sleep(0.1)
```

### **Test Video Capture:**
```python
import cv2

cap = cv2.VideoCapture("/dev/video0")
if cap.isOpened():
    print("Video device detected")
    ret, frame = cap.read()
    if ret:
        print("Video capture working")
    cap.release()
else:
    print("Video device not found")
```

## ⚠️ Important Notes

### **Power Requirements:**
- **Raspberry Pi 4:** 5V, 3A minimum
- **RX5808 Module:** 5V or 3.3V, ~100mA
- **USB Video DVR:** 5V, ~500mA
- **Total:** 5V, 3.5A recommended

### **Power Supply Options:**
- **Option 1:** Use 5V from Raspberry Pi (Pin 2)
- **Option 2:** Use 3.3V from Raspberry Pi (Pin 1) - safer for RX5808
- **Option 3:** Use external 5V supply with common ground

### **Heat Management:**
- RX5808 can get hot during operation
- Add heat sink if running continuously
- Ensure good ventilation

### **Antenna Considerations:**
- Use circular polarized antenna for FPV
- Match antenna frequency to 5.8GHz
- Position antenna for best reception

### **Interference:**
- Keep wires short and organized
- Avoid running signal wires near power
- Use shielded cables if possible

## 🚀 Troubleshooting

### **Common Issues:**

1. **SPI not working:**
   - Check `/boot/firmware/config.txt` settings
   - Verify pin connections (CH2=CS, A6.5M=MOSI, CH1=SCK)
   - Test with `ls /dev/spi*`

2. **RSSI readings erratic:**
   - Check RSSI pin connection
   - Verify ground connections
   - Test with multimeter

3. **Video not capturing:**
   - Check USB Video DVR connection
   - Verify device permissions
   - Test with `lsusb` command

4. **Poor signal reception:**
   - Check antenna connection to ANT pin
   - Verify antenna type and frequency
   - Test antenna positioning

5. **Power issues:**
   - Check voltage levels (5V or 3.3V)
   - Verify ground connections
   - Test with multimeter

### **Debug Commands:**
```bash
# Check SPI devices
ls /dev/spi*

# Check USB devices
lsusb

# Check video devices
ls /dev/video*

# Check GPIO
gpio readall

# Monitor system resources
htop

# Check power supply
vcgencmd measure_volts
```

### **Pin Verification:**
```bash
# Check if pins are accessible
gpio -g mode 8 out  # CH2 (CS)
gpio -g mode 10 out # A (MOSI)
gpio -g mode 9 in   # 6.5M (MISO)
gpio -g mode 11 out # CH1 (SCK)
gpio -g mode 7 in   # RSSI
```

---

**🎯 Your Raspberry Pi 4 RX5808 Scanner is ready to scan and capture video!**

**Key Points:**
- Use CH2 for Chip Select (CS)
- Use A for MOSI
- Use 6.5M for MISO  
- Use CH1 for SCK
- Use RSSI for signal strength
- Use VIDEO for video output
- Use ANT for antenna connection 