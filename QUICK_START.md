# Quick start

On a Raspberry Pi 4:

```bash
git clone https://github.com/nursekak/rpiskanC.git
cd rpiskanC
make install-deps
make setup-system
sudo reboot
```

After reboot:

```bash
cd rpiskanC
make
make test-hardware
./fpv_interceptor_gui
```

OpenCV preview: `make opencv` then `./fpv_interceptor_opencv`.

## pigpio

```bash
sudo systemctl enable pigpiod
sudo systemctl start pigpiod
sudo systemctl status pigpiod
```

## Wiring (short)

| RX5808 | Pi 4 |
|--------|------|
| GND | pin 6 |
| +5V or 3.3V | pin 2 (5V) or pin 1 (3.3V) — module accepts both; 5V is what the other wiring notes use |
| RSSI | pin 26 (GPIO 7) |
| VIDEO | USB capture analog in |
| MOSI (A / 6.5M side, see `RPI_WIRING.md`) | pin 19 (GPIO 10) |
| SCK (CH1) | pin 23 (GPIO 11) |
| CS (CH2) | pin 24 (GPIO 8) |
| ANT | 5.8 GHz antenna |

Full pinout: `QUICK_WIRING.md`, `RPI_WIRING.md`.

## What the binary does

Sweeps 5725–6000 MHz, 1 MHz step. Treats RSSI above 50 (header `RSSI_THRESHOLD`) as “occupied”. OpenCV target can grab `/dev/video0` when the analog path is plugged in.

Python helpers, if you want them: `examples/test_hardware.py`, `examples/simple_scanner.py`.

## Stuck

```bash
# pigpio
sudo systemctl start pigpiod
python3 -c "import pigpio; pi = pigpio.pi(); print('OK' if pi.connected else 'ERROR'); pi.stop()"

# SPI
echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
sudo reboot

# video
ls /dev/video*
lsusb
```
