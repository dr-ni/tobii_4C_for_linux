# tobii4c

Standalone Tobii Eye Tracker 4C support for Linux/X11.

The project provides low latency gaze tracking, head pose estimation,
blink detection and eye controlled mouse interaction for X11 desktops.

Features include:

* Tobii Engine systemd integration
* fullscreen X11 calibration
* standalone eye mouse
* realtime gaze tracking
* head pose tracking
* 3D gaze mapping
* blink click support
* barycentric mesh calibration
* adaptive edge reachability
* smoothing and drift reduction
* local per-user calibration storage
* optional AT-SPI accessibility integration

Tested on Ubuntu-based Linux systems using X11.

---

# Features

## Tobii Engine integration

The repository contains:

* `tobiiusb.service`
* `tobii_engine.service`

for standalone initialization of the Tobii runtime.

This avoids the need to manually start:

```text
TobiiProEyeTrackerManager
