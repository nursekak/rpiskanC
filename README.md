# rpiskanC

Raspberry Pi 4 path for **5.8 GHz analog video**: SPI control of an RX5808 receiver, RSSI scan across the band, GTK UI, optional OpenCV preview from a USB capture dongle.

C on the radio path. Python is not in the scan loop.

## What it actually does

Tune the RX5808 over SPI (CS / MOSI / MISO / SCK), read RSSI on GPIO 7, sweep **5725–6000 MHz** in 1 MHz steps (`FREQ_MIN` / `FREQ_MAX` / `FREQ_STEP` in `fpv_interceptor.h`). Threshold for “something is there” is `RSSI_THRESHOLD` (50 in that header) — not a calibrated dBm meter.

The GTK build (`make`) is the UI + scanner, with an RX5808 stub so you can compile off-Pi. The OpenCV build (`make opencv`) talks to a real driver and grabs frames from `/dev/video0` (640×480, 30 FPS in `video_detector.c`) after the analog out of the RX5808 goes into a USB video dongle.

Binary names from the Makefile are still `fpv_interceptor_gui` / `fpv_interceptor_opencv`. I did not rename the tree.

## Hardware

- Raspberry Pi 4, SPI on
- RX5808 5.8 GHz module
- USB analog-to-digital video dongle (only for the OpenCV target)

RX5808 → Pi:

| RX5808 | Pi |
|--------|----|
| VCC | 3.3 V (pin 1) |
| GND | GND (pin 6) |
| CS | GPIO 8 (pin 24) |
| MOSI | GPIO 10 (pin 19) |
| MISO | GPIO 9 (pin 21) |
| SCK | GPIO 11 (pin 23) |
| RSSI | GPIO 7 (pin 26) |
| VIDEO | USB dongle analog in |

More pin notes: `RPI_WIRING.md`, `QUICK_WIRING.md`.

## Build (on the Pi)

```bash
git clone https://github.com/nursekak/rpiskanC.git
cd rpiskanC
make install-deps
make setup-system   # enables SPI in config.txt, starts pigpiod; reboot after
sudo reboot
```

After reboot:

```bash
cd rpiskanC
make                # GTK UI → ./fpv_interceptor_gui
# or
make opencv         # GTK + OpenCV → ./fpv_interceptor_opencv
make test-hardware  # /dev/spi*, pigpiod, /dev/video*, lsusb
```

`make install-deps` pulls gcc, GTK3, OpenCV, v4l, and builds [pigpio](https://github.com/joan2937/pigpio) from source if needed.

## Layout

| File | Role |
|------|------|
| `fpv_gui_simple.c` | GTK UI used by the default target |
| `fpv_gui_opencv.cpp` | GTK + OpenCV UI |
| `rx5808_driver.c` / `rx5808_stub.c` | SPI tuner vs compile-without-radio stub |
| `rssi_analyzer.c` | RSSI samples / history |
| `frequency_scanner_fixed.c` | sweep |
| `video_detector.c` | OpenCV capture from `/dev/video0` |
| `Makefile` | `all`, `opencv`, `install-deps`, `setup-system`, `test-hardware` |

## Limits

This is a receiver + RSSI plot + preview on a Pi. No radio TX, no flight controller, no network stack. RSSI is the module’s analog pin scaled to 0–100, not a lab instrument. I did not put made-up sweep-rate or “±2% accuracy” numbers in here — they are not measured in-repo.
