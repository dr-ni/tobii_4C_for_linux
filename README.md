# Tobii 4C for Linux

Standalone Tobii 4C eye tracking support for Linux/X11 with:

* Tobii Engine systemd integration
* fullscreen X11 calibration
* standalone eye mouse
* 3x3 mesh calibration
* barycentric warp correction
* adaptive edge reachability
* smoothing and drift reduction
* local per-user calibration storage

Tested on Ubuntu-based systems using X11.

---

# Features

## Tobii Engine integration

The repository contains:

* `tobiiusb.service`
* `tobii_engine.service`

for proper standalone initialization of the Tobii runtime.

This avoids the need to start:

* TobiiProEyeTrackerManager

before using gaze tracking.

---

# Included tools

## calibration_x11

Fullscreen calibration utility.

Features:

* borderless fullscreen mode
* no window manager interference
* 3x3 calibration mesh
* red/green calibration targets
* retry handling
* dynamic screen resolution
* local calibration storage

Calibration data is stored in:

```text
~/.local/tobii_4c/calx11.conf
```

---

## eye_mouse_x11

Standalone X11 eye mouse.

Features:

* barycentric mesh warp
* triangle interpolation
* adaptive edge correction
* smoothing
* drift reduction
* dynamic screen resolution
* standalone Tobii Engine support

Uses calibration from:

```text
~/.local/tobii_4c/calx11.conf
```

---

# Requirements

Ubuntu packages:

```bash
sudo apt install \
libx11-dev \
build-essential \
libsqlcipher1
```

Some Ubuntu versions require compatibility symlink:

```bash
sudo ln -s \
/lib/x86_64-linux-gnu/libsqlcipher.so.1 \
/lib/x86_64-linux-gnu/libsqlcipher.so.0
```

---

# Installation

## Install Tobii services

Install repository services:

```bash
chmod +x install.sh
sudo ./install.sh
```

---

## Enable services

```bash
sudo systemctl daemon-reload

sudo systemctl enable tobiiusb
sudo systemctl enable tobii_engine

sudo systemctl start tobiiusb
sudo systemctl start tobii_engine
```

---

# Verify services

```bash
systemctl status tobiiusb
systemctl status tobii_engine
```

Expected:

```text
active (running)
```

---

# Build

## Build all

```bash
make
```

---

## Install binaries and man pages

```bash
sudo make install
```

Installed binaries:

```text
/usr/local/bin/calibration_x11
/usr/local/bin/eye_mouse_x11
```

Installed man pages:

```text
man calibration_x11
man eye_mouse_x11
```

---

# Calibration

Run fullscreen calibration:

```bash
calibration_x11
```

or locally:

```bash
./calibration_x11
```

The calibration file is automatically written to:

```text
~/.local/tobii_4c/calx11.conf
```

---

# Eye mouse

Start the standalone eye mouse:

```bash
eye_mouse_x11
```

or locally:

```bash
./eye_mouse_x11
```

---

# Recalibration

If cursor accuracy becomes worse:

```bash
calibration_x11
```

again.

---

# Clean build

```bash
make clean
```

---

# Uninstall

```bash
sudo make uninstall
```

Repository uninstall:

```bash
chmod +x uninstall.sh
sudo ./uninstall.sh
```

---

# Notes

## X11 required

Current implementation uses:

* XWarpPointer
* X11 fullscreen calibration

and therefore requires:

```text
X11 session
```

Wayland is currently unsupported.

---

## Tobii Engine warnings

You may see warnings such as:

```text
Bad key!
ODIN_ERROR_INTERNAL
```

inside `tobii_engine.service` logs.

These are typically related to:

* telemetry
* licensing
* cloud components

and usually do not affect local gaze tracking.

---
##### Disclaimer

I do not have any relationship with Tobii and I do not own the libraries and the tools provided here. As far as I know, they have been made [publicly available](https://developer.tobii.com/community/forums/topic/tobii-4c-eye-tracker-on-linux/) by Tobii and you should check with them about licensing.

##### Acknowledgments

Special thanks to:
* [@Ejohngebbie](https://github.com/johngebbie) for builing the previous tobii_4C_for_linux project
* [@Eitol](https://github.com/Eitol) for putting together all the necessary tools from Tobii.
* Tobii for making Linux Tobii experimentation possible.

