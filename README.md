# tobii4c

Standalone Tobii Eye Tracker 4C support for Linux/X11.

`tobii4c` provides realtime gaze tracking, fullscreen calibration
and eye-controlled mouse interaction for Linux desktops using X11.

The current stable runtime is based on:

* barycentric triangle mesh calibration
* 3x3 calibration mesh warping
* Tobii Stream Engine
* direct X11 cursor control
* low latency gaze smoothing
* local triangle interpolation
* optional Quha gyro integration

The current runtime does NOT yet use:

* full projective warp
* active bootstrap geometry
* active C12 mapping
* full 3D reprojection

These parts exist experimentally in newer branches but are not part
of the current stable runtime.

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
triangle selection
        │
        ▼
barycentric interpolation
        │
        ▼
local warped coordinates
        │
        ▼
runtime smoothing
        │
        ▼
optional Quha gyro offset
        │
        ▼
X11 cursor mapping
        │
        ▼
XWarpPointer
```

---

# Installation

## 1. Install Tobii services and libraries

```bash
bash install.sh
```

This installs:

- Tobii USB service (`tobiiusb.service`)
- Tobii Stream Engine service (`tobii_engine.service`)
- Tobii shared libraries into `/usr/lib/tobii/`
- Tobii headers into `/usr/include/tobii/`
- Library config into `/etc/ld.so.conf.d/tobii.conf`

Both systemd services are enabled and started automatically.

## 2. Build eyecalib and eyemouse

```bash
make
```

## 3. Install binaries and man pages

```bash
sudo make install
```

Installs to `/usr/local/bin/` by default.
A custom prefix can be set:

```bash
sudo make install PREFIX=/usr
```

## 4. Run calibration

```bash
eyecalib
```

## 5. Run eye mouse

```bash
eyemouse
```

---

# Uninstall

Remove Tobii services and libraries:

```bash
bash uninstall.sh
```

Remove binaries and man pages:

```bash
sudo make uninstall
```

---

# Calibration

## eyecalib

`eyecalib` displays a fullscreen calibration overlay and collects
gaze samples for 9 screen positions arranged in a 3x3 grid.

Run:

```bash
eyecalib
```

The calibration process:

```text
fullscreen targets
    -> gaze sample collection (250 samples per point)
    -> invalid sample rejection
    -> gaze averaging
    -> calibration mesh generation
    -> persistent calibration storage
```

Calibration data is stored in:

```
~/.local/tobii_4c/calx11.conf
```

### Tobii DB bootstrap

If a Tobii Pro Eye Tracker Manager profile exists at:

```
~/.config/TobiiProEyeTrackerManager/db/da-setups.db
```

`eyecalib` will automatically import physical screen geometry
(tracker placement, screen tilt, display dimensions) and derive
a `sensor_offset_y` correction from it.

### Calibration targets

The 9 targets are placed near screen edges to maximize
fullscreen reachability:

```text
P00(5%,5%)   P01(50%,5%)   P02(95%,5%)
P10(5%,50%)  P11(50%,50%)  P12(95%,50%)
P20(5%,95%)  P21(50%,95%)  P22(95%,95%)
```

### Calibration file format

Each line stores one calibration point:

```
raw_x raw_y target_x target_y
```

Example:

```
0.052524 0.075656 0.050000 0.050000
```

| Value    | Meaning                    |
|----------|----------------------------|
| raw_x    | measured Tobii gaze x      |
| raw_y    | measured Tobii gaze y      |
| target_x | expected screen x (0–1)    |
| target_y | expected screen y (0–1)    |

The config file also stores runtime parameters:

| Parameter      | Default | Meaning                          |
|----------------|---------|----------------------------------|
| gaze_smooth    | 0.12    | gaze low-pass filter strength    |
| cursor_smooth  | 0.22    | cursor low-pass filter strength  |
| edge_zone      | 0.08    | edge expansion zone width        |
| edge_power     | 1.35    | edge expansion nonlinearity      |
| sensor_offset_x| 0.0     | horizontal sensor offset         |
| sensor_offset_y| 0.0     | vertical sensor offset           |
| screen_distance| 63.0    | screen distance in cm            |
| screen_tilt    | 0.0     | screen tilt angle in degrees     |

---

# Run Eye Mouse

## eyemouse

```bash
eyemouse [options] [config_file]
```

### Options

| Option      | Description                              |
|-------------|------------------------------------------|
| `-h --help` | show help                                |
| `--usegyro` | enable Quha gyroscopic fine correction   |

### Examples

```bash
# default calibration
eyemouse

# custom calibration file
eyemouse ~/.local/tobii_4c/calx11.conf

# with Quha gyro
eyemouse --usegyro

# gyro with custom calibration
eyemouse --usegyro tobii.conf
```

### Example output

```
Screen: 1920x1080
Device: tobii-prp://TOBIIDFU4C-090200627319
Gyro disabled
Mouse 1365  150
```

---

# 3x3 Barycentric Calibration Mesh

The 9 calibration points form a triangulated mesh:

```text
P00  P01  P02
P10  P11  P12
P20  P21  P22
```

The mesh is split into 8 triangles. For every gaze sample:

1. the containing triangle is selected
2. barycentric weights are calculated
3. local warped screen coordinates are interpolated

This improves:

- fullscreen reachability
- edge precision
- corner access
- asymmetric distortion correction
- local edge compensation

Without mesh correction, raw gaze mapping typically causes:

- compressed edges
- unstable corners
- asymmetric fullscreen reachability
- poor border access

---

# Runtime Smoothing

The runtime applies two-stage exponential smoothing:

```text
gaze_smooth   = 0.12   (gaze low-pass)
cursor_smooth = 0.22   (cursor low-pass)
```

A deadzone of 3 pixels suppresses micro-jitter when the
eye is stationary.

This improves:

- cursor stability
- accessibility usage
- fullscreen usability
- small target interaction

---

# Optional Quha Gyro Support

`eyemouse --usegyro` enables Quha gyroscopic input for fine
cursor correction on top of the Tobii gaze position.

The Quha device is automatically detected via:

```
/dev/input/by-id/
```

Gyro movement is applied as a relative offset that decays
exponentially (factor 0.92 per frame), so gaze remains the
primary positioning input.

---

# X11 Requirement

Both tools require an active X11 session.
Wayland is currently not supported.

Verify your session type:

```bash
echo $XDG_SESSION_TYPE
# must output: x11
```

---

# Calibration

## Recalibrate

Simply run `eyecalib` again. The previous calibration file
will be overwritten.

## Manual edit

The calibration file is plain text and can be edited directly:

```
~/.local/tobii_4c/calx11.conf
```

---

# Experimental / Incomplete Features

Partially implemented or experimental:

- bootstrap geometry import
- C12 runtime
- 3D gaze reprojection
- head compensated mapping
- projective warp
- adaptive mesh refinement
- binocular convergence estimation
- advanced geometry correction

These are not fully active in the stable runtime.

---

# Future Direction

Planned future improvements:

- projective warp / homography
- bilinear mesh interpolation
- geometry-aware reprojection
- headpose compensation
- dynamic runtime calibration
- adaptive edge expansion
- improved corner precision

---

# Files

| Path | Description |
|------|-------------|
| `~/.local/tobii_4c/calx11.conf` | calibration and runtime config |
| `/usr/lib/tobii/` | Tobii Stream Engine shared libraries |
| `/usr/include/tobii/` | Tobii Stream Engine headers |
| `/etc/ld.so.conf.d/tobii.conf` | library path config |
| `/etc/systemd/system/tobii_engine.service` | Tobii engine service |
| `/etc/systemd/system/tobiiusb.service` | Tobii USB service |

---

# License

MIT-style open source / research-oriented project.
