# tobii4c

Standalone Tobii Eye Tracker 4C support for Linux/X11.

`tobii4c` provides realtime gaze tracking, fullscreen calibration,
eye controlled mouse interaction and experimental geometry-aware
3D gaze processing for Linux desktops.

The project focuses on:

- accessibility
- assistive input
- low latency gaze interaction
- standalone Linux eye tracking
- experimental HCI
- geometry-aware gaze mapping
- realtime filtering and stabilization

Designed primarily for:

- hands-free desktop interaction
- accessibility workflows
- Linux eye tracking research
- assistive desktop environments
- experimental gaze interaction

Tested primarily on Ubuntu-based Linux systems using X11.

---

# Components

| Component | Description |
|---|---|
| eyemouse | realtime eye-controlled mouse |
| eyecalib | fullscreen gaze calibration |
| tobiiusb.service | Tobii USB runtime initialization |
| tobii_engine.service | Tobii Stream Engine runtime |

---

# Requirements

## Runtime

Required:

- Linux
- X11 session
- Tobii Eye Tracker 4C
- Tobii Stream Engine runtime
- X11 libraries

## Development

Required for compilation:

- gcc
- make
- libx11-dev
- Tobii Stream Engine SDK

Ubuntu example:

```bash
sudo apt install \
    build-essential \
    libx11-dev
