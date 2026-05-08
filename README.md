# tobii4c

Standalone Tobii Eye Tracker 4C support for Linux/X11.

`tobii4c` provides low latency gaze tracking, head pose estimation,
blink detection and eye controlled mouse interaction for Linux desktops.

The project focuses on:

* standalone operation
* X11 integration
* accessibility
* assistive interaction
* low latency tracking
* realtime filtering
* adaptive calibration
* geometry aware gaze mapping

The software is designed primarily for:

* accessibility workflows
* hands-free desktop interaction
* Linux eye tracking research
* gaze interaction experiments
* Tobii reverse engineering
* assistive desktop environments

Tested primarily on Ubuntu-based Linux systems using X11.

---

# Current Status

Implemented:

* realtime Tobii gaze tracking
* fullscreen X11 calibration
* local calibration persistence
* adaptive edge compensation
* Tobii geometry bootstrap
* gaze filtering and smoothing
* X11 eye mouse integration
* head pose capable runtime pipeline

Experimental:

* blink click
* dwell click
* 3D gaze mapping
* head compensated tracking
* barycentric warp refinement

---

# Requirements

## Runtime

* Linux
* X11 session
* Tobii Eye Tracker 4C
* Tobii Stream Engine runtime
* Xlib

## Development

* gcc
* make
* X11 development headers
* Tobii Stream Engine SDK

Ubuntu example:

```bash
sudo apt install \
    build-essential \
    libx11-dev
