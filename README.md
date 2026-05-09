# tobii4c

Standalone Tobii Eye Tracker 4C support for Linux/X11.

`tobii4c` provides realtime gaze tracking, fullscreen calibration
and eye-controlled mouse interaction for Linux desktops using X11.

The current stable runtime is based on:

* affine gaze calibration
* 3x3 calibration mesh
* Tobii Stream Engine
* direct X11 cursor control
* low latency gaze smoothing

The current runtime does NOT yet use:

* full projective warp
* active bootstrap geometry
* active C12 mapping
* full 3D reprojection

These parts exist experimentally in newer branches but are not part
of the current stable runtime.

---

# Components

| Component            | Description                    |
| -------------------- | ------------------------------ |
| eyemouse             | realtime gaze-controlled mouse |
| eyecalib             | fullscreen X11 calibration     |
| tobiiusb.service     | Tobii USB initialization       |
| tobii_engine.service | Tobii Stream Engine runtime    |

---

# Requirements

## Runtime

Required:

* Linux
* X11 session
* Tobii Eye Tracker 4C
* Tobii Stream Engine runtime
* X11 libraries

Wayland is currently unsupported.

Check:

```bash
echo $XDG_SESSION_TYPE
```

Expected:

```text
x11
```

---

# Build

```bash
make
```

---

# Install

```bash
sudo make install
```

---

# Runtime Architecture

Current stable runtime pipeline:

```text
Tobii Eye Tracker 4C
        │
        ▼
Tobii Stream Engine
        │
        ▼
raw gaze coordinates
        │
        ▼
affine calibration transform
        │
        ▼
runtime smoothing
        │
        ▼
edge stabilization
        │
        ▼
X11 cursor mapping
        │
        ▼
XWarpPointer
```

---

# Calibration

Calibration data is stored in:

```text
~/.local/tobii_4c/calx11.conf
```

Current stable runtime expects:

```text
9 calibration points
```

stored as:

```text
raw_x raw_y target_x target_y
```

Example:

```text
0.052524 0.075656 0.050000 0.050000
```

Meaning:

| Value    | Meaning               |
| -------- | --------------------- |
| raw_x    | measured Tobii gaze x |
| raw_y    | measured Tobii gaze y |
| target_x | expected screen x     |
| target_y | expected screen y     |

---

# Stable Runtime Calibration Format

The current stable runtime reads:

```text
0.052524 0.075656 0.050000 0.050000
0.498691 0.050737 0.500000 0.050000
0.941116 0.014396 0.950000 0.050000
0.064467 0.501589 0.050000 0.500000
0.502002 0.497415 0.500000 0.500000
0.945250 0.498374 0.950000 0.500000
0.052460 0.932928 0.050000 0.950000
0.503232 0.971103 0.500000 0.950000
0.946206 0.936097 0.950000 0.950000
```

The stable runtime currently ignores:

* sensor_offset_x
* sensor_offset_y
* screen_distance
* screen_tilt
* bootstrap geometry
* C12 parameters

These fields belong to experimental newer runtime branches.

---

# 3x3 Calibration Mesh

The calibration points form a 3x3 mesh:

```text
P00  P01  P02
P10  P11  P12
P20  P21  P22
```

The stable runtime derives an affine correction transform from the
measured gaze space.

This improves:

* fullscreen reachability
* edge precision
* corner access
* asymmetric distortion

---

# Affine Runtime Mapping

The stable runtime uses affine mapping:

x:

```text
x' = ax + by + c
```

y:

```text
y' = dx + ey + f
```

This is significantly more stable than direct raw gaze mapping.

Without affine correction:

```text
raw gaze → direct screen mapping
```

typically causes:

* compressed edges
* unstable corners
* asymmetric fullscreen reachability

---

# Edge Behavior

Eye trackers are nonlinear devices.

Typical behavior:

```text
center:
    stable

edges:
    compressed

corners:
    less accurate
```

The affine runtime compensates part of this distortion.

---

# Runtime Smoothing

The runtime performs:

* exponential smoothing
* gaze stabilization
* jitter reduction
* motion damping

This improves:

* cursor stability
* accessibility usage
* fullscreen usability
* small target interaction

---

# Fullscreen Calibration

Run:

```bash
eyecalib
```

Features:

* fullscreen overlay
* edge targets
* realtime gaze sampling
* invalid sample rejection
* persistent calibration storage

---

# Run Eye Mouse

```bash
eyemouse
```

Example output:

```text
Mouse 1365 150
```

Meaning:

| Value | Meaning           |
| ----- | ----------------- |
| 1365  | cursor x position |
| 150   | cursor y position |

---

# Current Stable Runtime Capabilities

Implemented and working:

* Tobii gaze tracking
* realtime cursor control
* affine calibration mapping
* 3x3 calibration mesh
* fullscreen X11 calibration
* low latency filtering
* fullscreen edge reachability

---

# Experimental / Incomplete Features

Partially implemented or experimental:

* bootstrap geometry import
* C12 runtime
* 3D gaze reprojection
* head compensated mapping
* projective warp
* adaptive mesh refinement
* binocular convergence estimation
* advanced geometry correction

These are not fully active in the stable runtime.

---

# Current Recommended Architecture

Recommended stable setup:

```text
eye_mouse_stable.c
    +
calx11 3x3 affine calibration
```

This currently provides the best balance between:

* stability
* fullscreen reachability
* low latency
* predictable behavior
* accessibility usability

---

# Limitations

Current limitations:

* X11 only
* no Wayland support
* no full 3D geometry correction
* no active headpose reprojection
* no multi-user support

---

# Future Direction

Planned future improvements:

* projective warp / homography
* bilinear mesh interpolation
* geometry-aware reprojection
* headpose compensation
* dynamic runtime calibration
* adaptive edge expansion
* improved corner precision

---

# License

MIT-style open source / research-oriented project.

