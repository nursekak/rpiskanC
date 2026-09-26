# Quick wiring — RX5808 on a Pi 4

Analog VIDEO from the module does not go into a Pi CSI port. It goes into a USB capture dongle, then `/dev/video0`.

```
RX5808          Pi 4                 Role
GND          →  pin 6 GND
+5V          →  pin 2 5V             (3.3 V pin 1 also works; 5 V is what I used in the other notes)
RSSI         →  pin 26 GPIO 7
VIDEO        →  USB capture analog in
A / MOSI     →  pin 19 GPIO 10
CH1 / SCK    →  pin 23 GPIO 11
CH2 / CS     →  pin 24 GPIO 8
ANT          →  5.8 GHz antenna
USB capture  →  Pi USB
```

Header `fpv_interceptor.h` maps CS 8, MOSI 10, MISO 9, SCK 11, RSSI 7 (BCM). MISO is GPIO 9 (pin 21) — do not skip it even if a cheap pinout drawing forgets the label.

## Check

```bash
ls /dev/spi*
ls /dev/video*
python3 examples/test_hardware.py
```

SPI:

```bash
sudo nano /boot/firmware/config.txt
# dtparam=spi=on
sudo reboot
```

Then `make` / `./fpv_interceptor_gui`. Python `src/simple_scanner.py` is not in this tree; examples live under `examples/`.

Common ground. Short wires. Antenna on ANT, 5.8 GHz.
