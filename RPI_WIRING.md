# Raspberry Pi 4 + RX5808 wiring

Receiver module on SPI, RSSI on a GPIO, analog video through a USB capture dongle. Not a CSI camera board.

## Parts

- Pi 4 (4 GB is what I used)
- RX5808 5.8 GHz receiver
- 5.8 GHz antenna on ANT
- USB analog capture dongle if you want OpenCV preview
- 5 V supply, 3 A-class for the Pi

MCP3008 / extra ADCs are optional. The C code reads RSSI on GPIO 7 as in `fpv_interceptor.h`.

## RX5808 → Pi

| RX5808 | Function | Pi 4 |
|--------|----------|------|
| GND | ground | pin 6 GND |
| +5V | power | pin 2 5V (pin 1 3.3V is safer for some modules) |
| RSSI | analog RSSI | GPIO 7 / pin 26 |
| VIDEO | analog out | USB capture **input** |
| A (MOSI) | SPI MOSI | GPIO 10 / pin 19 |
| 6.5M (MISO) | SPI MISO | GPIO 9 / pin 21 |
| CH1 | SPI SCK | GPIO 11 / pin 23 |
| CH2 | SPI CS | GPIO 8 / pin 24 |
| CH3 | unused | — |
| ANT | antenna | 5.8 GHz antenna |

`VIDEO → USB dongle → Pi USB → /dev/video0`.

## Enable SPI

```bash
sudo nano /boot/firmware/config.txt
# dtparam=spi=on
# dtoverlay=spi0-2cs
sudo reboot
```

## Power

Pi 4 wants a proper 5 V / 3 A PSU. RX5808 is ~100 mA. Dongle is extra USB current. Common ground if you use an external 5 V for the module. It runs warm; a small heatsink is enough for long scans.

## Test

```bash
ls /dev/spi*
lsusb
ls /dev/video*
python3 examples/test_hardware.py
```

SPI smoke (Python), optional:

```python
import spidev, RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setup(8, GPIO.OUT)
spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 2000000
GPIO.output(8, GPIO.LOW)
spi.writebytes([0x01, 0x02])
GPIO.output(8, GPIO.HIGH)
print("SPI write done")
```

OpenCV:

```python
import cv2
cap = cv2.VideoCapture("/dev/video0")
print("opened" if cap.isOpened() else "no /dev/video0")
cap.release()
```

## If it is dead

- no `/dev/spi*`: config.txt + reboot
- garbage RSSI: ground and RSSI pin
- no picture: dongle, `lsusb`, `video` group
- weak receive: antenna on ANT, 5.8 GHz, placement
