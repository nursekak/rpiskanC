# Usage

Two Makefile targets. Binary names are unchanged.

## GTK only (no camera)

```bash
make
./fpv_interceptor_gui
```

RSSI scan and the live plot. No `/dev/video0`.

## GTK + OpenCV preview

```bash
chmod +x install_opencv.sh
./install_opencv.sh   # or: sudo apt install -y libopencv-dev python3-opencv
make opencv
./fpv_interceptor_opencv
```

Needs the analog VIDEO pin on the RX5808 into a USB capture dongle, then that dongle on the Pi.

```
RX5808 VIDEO  →  USB capture analog in
USB capture   →  Pi USB  →  /dev/video0
```

Check:

```bash
lsusb | grep -i video
ls -la /dev/video*
```

Saves under `captures/` (`video_*.avi` from `video_detector.c`). Signal dumps and `logs/` if the UI writes them.

## GUI

Scan sweeps 5725–6000 MHz. Monitor locks one frequency. RSSI bar is 0–100 from the module pin, not dBm. Preview is the OpenCV target only.

## If it does not start

```bash
pkg-config --modversion opencv4 || pkg-config --modversion opencv
ls -la /dev/video*
make test-hardware
make clean && make opencv
sudo systemctl status pigpiod
```

`journalctl -u fpv-interceptor` only exists if you installed that unit (`make install-service`). Default run is just the binary.
