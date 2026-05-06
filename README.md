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

The software is designed primarily for:

* accessibility workflows
* hands-free desktop interaction
* Linux eye tracking research
* gaze interaction experiments
* Tobii reverse engineering
* assistive desktop environments

Tested primarily on Ubuntu-based Linux systems using X11.

---

# Features

## Core tracking

* realtime gaze tracking
* binocular eye tracking
* gaze vector filtering
* adaptive smoothing
* drift reduction
* low latency processing
* realtime coordinate mapping

---

## Head tracking

The tracking pipeline can process:

* head X/Y/Z position
* pitch
* yaw
* roll
* eye openness
* movement stability

This enables:

* dynamic gaze correction
* future head compensated tracking
* blink detection
* dwell interaction
* gaze stability estimation

---

## Eye mouse

The included `eyemouse` utility provides:

* eye controlled mouse movement
* gaze-to-pointer mapping
* adaptive edge reachability
* smoothing and stabilization
* dynamic screen scaling
* multi-monitor support
* optional accessibility integration

The eye mouse is optimized for:

* desktop accessibility
* low effort pointer control
* reduced jitter
* improved edge access
* long-term usage stability

---

## Calibration

The included `eyecalib` utility provides:

* fullscreen calibration
* unmanaged fullscreen mode
* 3x3 calibration mesh
* barycentric warp preparation
* realtime gaze visualization
* adaptive edge compensation
* persistent local calibration storage

Calibration data is stored locally per user.

---

# Architecture

```text
Tobii Eye Tracker 4C
        │
        ▼
Tobii Stream Engine
        │
        ▼
tobii4c tracking pipeline
        │
        ├── gaze tracking
        ├── filtering
        ├── smoothing
        ├── head pose
        ├── blink detection
        ├── calibration warp
        │
        ▼
X11 integration layer
        │
        ▼
Desktop pointer control
