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

# 3x3 Barycentric Calibration Mesh

The calibration points form a triangulated 3x3 mesh:

```text
P00  P01  P02
P10  P11  P12
P20  P21  P22
```

The stable runtime splits the mesh into 8 triangles.

For every gaze sample:

1. the containing triangle is selected
2. barycentric weights are calculated
3. local warped screen coordinates are interpolated

This improves:

* fullscreen reachability
* edge precision
* corner access
* asymmetric distortion correction
* local edge compensation

---

# Barycentric Triangle Interpolation

The stable runtime uses local triangle interpolation instead of a
single global affine transform.

Conceptually:

```text
raw gaze
    -> triangle selection
    -> barycentric coordinates
    -> local warped interpolation
```

This behaves significantly better than direct raw gaze mapping.

Without mesh correction:

```text
raw gaze -> direct screen mapping
```

typically causes:

* compressed edges
* unstable corners
* asymmetric fullscreen reachability
* poor border access

---

# Runtime Warp Function

The current stable runtime internally uses:

* `warp()`
* `bary()`
* local triangle meshes
* barycentric coordinates

The runtime currently contains 8 local interpolation triangles.

This is significantly more advanced than direct affine scaling.

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

The barycentric mesh runtime compensates a significant part of this
behavior using local triangle interpolation.

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

---

# Current Stable Runtime Capabilities

Implemented and working:

* Tobii gaze tracking
* realtime cursor control
* barycentric triangle mesh mapping
* 3x3 calibration mesh
* fullscreen X11 calibration
* low latency filtering
* fullscreen edge reachability
* local triangle warping

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
3x3 barycentric calibration mesh
```

This currently provides the best balance between:

* stability
* fullscreen reachability
* low latency
* predictable behavior
* accessibility usability

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

