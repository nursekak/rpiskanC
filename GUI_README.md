# GUI

GTK+3 on Linux. Default `make` builds `./fpv_interceptor_gui` from `fpv_gui_simple.c`. `make opencv` builds `./fpv_interceptor_opencv` (GTK + `/dev/video0`).

There is no `make gui` target. `make` / `make all` is the GTK binary.

```bash
sudo apt install -y libgtk-3-dev libcairo2-dev libpango1.0-dev
# or
make install-deps

make
./fpv_interceptor_gui
```

Launcher script, if you generated it: `./fpv_gui.sh`.

## Layout

- Top: analog preview when the OpenCV binary is running and a capture dongle is present
- Controls: start/stop sweep, lock one frequency
- RSSI bar 0–100 and a short history plot

Sweep range is 5725–6000 MHz. Window sizes in `fpv_gui.c` (`GUI_WIDTH` 1200, `GUI_HEIGHT` 800, video 800×400) if that file is the one you compiled against; the simple target is `fpv_gui_simple.c`.

## Remote display

```bash
ssh -X user@pi-hostname
./fpv_interceptor_gui
```

VNC works too; that is the Pi desktop, not something special in this repo.

## GUI will not start

```bash
pkg-config --modversion gtk+-3.0
make clean && make
ls /dev/video*          # preview only
htop                    # if the plot stutters
```
