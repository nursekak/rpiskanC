# Install

Pi 4, Raspberry Pi OS. RX5808 + 5.8 GHz antenna. USB analog capture dongle only if you want OpenCV preview. PSU: 5 V, 3 A-class for the board.

The C tree uses **pigpio + GTK3**, not WiringPi. Python 3 shows up in `examples/`, not in the scan loop.

## Fast path

```bash
sudo apt update && sudo apt upgrade -y
git clone https://github.com/nursekak/rpiskanC.git
cd rpiskanC
make install-deps
make setup-system
sudo reboot
```

Then:

```bash
cd rpiskanC
make                 # ./fpv_interceptor_gui
make create-dirs
make test-hardware
```

Preview: `make opencv` → `./fpv_interceptor_opencv`.

`make setup-system` appends SPI lines to `/boot/firmware/config.txt` and enables `pigpiod`. Reboot is required.

## Manual SPI (if you skip setup-system)

```bash
echo "dtparam=spi=on" | sudo tee -a /boot/firmware/config.txt
echo "dtoverlay=spi0-2cs" | sudo tee -a /boot/firmware/config.txt
sudo reboot
```

```bash
ls /dev/spi*
ls /dev/video*
python3 examples/test_hardware.py
```

Groups if `/dev/spidev*` or GPIO is permission-denied:

```bash
sudo usermod -aG gpio,spi,video "$USER"
sudo reboot
```

## Optional systemd unit

Only if you actually ran `make install-service` (unit name `fpv-interceptor`):

```bash
make install-service
make start-service
sudo systemctl status fpv-interceptor
```

Otherwise just run the binary.

## Config in code, not a mystery .conf

Frequencies and RSSI live in `fpv_interceptor.h`: `FREQ_MIN` 5725, `FREQ_MAX` 6000, `FREQ_STEP` 1, `RSSI_THRESHOLD` 50, `RSSI_SAMPLES` 100. Capture size is 640×480 @ 30 in `video_detector.c`.

## Wiring

See `RPI_WIRING.md`. Short version: SPI CS/MOSI/MISO/SCK, RSSI on GPIO 7, analog VIDEO into the USB dongle.

## Build errors

```bash
sudo apt install -y build-essential cmake pkg-config libgtk-3-dev libopencv-dev
make clean
make
```
