# Graphics in Multimedia — Learning Roadmap (Embedded Linux Focus)

This document is a scratch-to-advanced roadmap for **computer graphics** in **multimedia**, with emphasis on **Embedded Linux** and **system-level** behavior (framebuffers, DRM, pixel formats, DMA, etc.).

---

## === LEARNING ROADMAP ===

### Phase 0 — Prerequisites (1–2 weeks)
- **C**: pointers, structs, `mmap`, file I/O, bit operations.
- **Linux userspace**: processes, `/dev` nodes, `ioctl`, basic kernel vs userspace boundary.
- **Math refresh**: linear algebra (vectors, matrices), basic trigonometry.

### Phase 1 — Foundations (2–3 weeks)
- Raster vs vector; pixels; resolution; color depth.
- RGB, YUV families; why video uses YUV.
- Framebuffer mental model: a memory-backed 2D array of pixels.

### Phase 2 — Core graphics (3–4 weeks)
- 2D: lines (Bresenham), rectangles, affine transforms.
- 3D intro: model/view/projection; orthographic vs perspective.
- Display path: scan-out, refresh rate, tearing, double buffering, VSync.

### Phase 3 — Multimedia bridge (2–3 weeks)
- Video vs graphics; decoding vs compositing vs presentation.
- YUV420, NV12; stride, alignment; scaling and color conversion (CPU vs GPU/ISP).

### Phase 4 — Linux graphics stack (3–5 weeks)
- `/dev/fb0` legacy framebuffer.
- DRM/KMS: connectors, CRTCs, planes, atomic modesetting.
- Wayland vs X11; when you still see framebuffer-only systems.

### Phase 5 — Embedded (3–4 weeks)
- SPI TFT (e.g. ILI9341, ST7789): partial updates, SPI bandwidth.
- DMA for display/memory moves; cache coherency awareness (platform-dependent).
- Power, thermal, and real-time constraints.

### Phase 6 — APIs & tools (ongoing)
- **OpenGL ES** (embedded 3D), **Cairo** (2D vector), **SDL** (window/input + portability), **FFmpeg** (demux/decode → raw frames).

### Phase 7 — Capstone (4+ weeks)
- Image viewer → video player (software path) → optimize with GPU/DRM where available → optional SPI display port of a subset.

**Suggested pace:** ~6–12 months part-time for solid intermediate; “advanced” adds GPU shaders, display controllers in depth, and vendor BSP work.

---

## === CORE CONCEPT EXPLANATION ===

### 1) What is computer graphics?
**Computer graphics** is the discipline of **representing, manipulating, and displaying** visual information using computers. In **multimedia**, it overlaps with **video** (time-sequenced frames) and **UI/compositing** (layers, text, effects).

### 2) Types of graphics

| Type | Description | Typical use |
|------|-------------|-------------|
| **Raster** | Image as a **grid of pixels** (samples). Scaling can blur or alias. | Photos, video frames, framebuffer contents. |
| **Vector** | **Geometric primitives** (paths, Bézier curves) + fill/stroke rules. Infinitely scalable until rasterized. | Fonts, icons, PDF, UI at multiple DPI. |

### 3) Pixel, resolution, color depth
- **Pixel**: smallest addressable picture element; has a **color value** (and sometimes alpha).
- **Resolution**: e.g. **1920×1080** = width × height in pixels.
- **Color depth**: bits per pixel (bpp) or per channel.
  - **8 bpp** (palette / indexed) — rare today for main displays.
  - **16 bpp** (often RGB565) — common on cheap panels/MCUs.
  - **24 bpp** (RGB888) — truecolor without alpha.
  - **32 bpp** (RGBA / XRGB) — adds alpha or unused padding for alignment.

**Stride (pitch)**: bytes per **row**, often **≥** `width * bytes_per_pixel` due to **alignment**; always use stride when indexing rows.

### 4) Framebuffer concept
A **framebuffer** is a **memory region** holding the **image currently (or soon) scanned out** to a display. Conceptually:

```
Application writes pixels → memory (framebuffer) → display controller reads → panel
```

On Linux, **userspace** may map GPU memory or a dumb framebuffer via `mmap` and draw with CPU, or submit buffers through higher-level APIs.

### 5) RGB vs YUV color formats

| Family | Idea | Common in |
|--------|------|-----------|
| **RGB** | Red, Green, Blue **additive** primaries; natural for **screens**. | UI, OpenGL textures (often), some cameras. |
| **YUV / YCbCr** | **Luma (Y)** + **chrominance (U/V)**; separates **brightness** from **color**. | **Video compression** (JPEG/MPEG/H.264), cameras, TVs. |

Why **YUV for video**: human vision is **more sensitive to luma** than chroma → codecs **subsample chroma** (e.g. 4:2:0) for huge bitrate savings.

### 6) Rendering pipeline (high level)
Typical **3D** path (conceptually):
1. **Vertices** in model space → **world** → **camera (view)** space.
2. **Projection** to clip space → **rasterization** produces fragments (candidate pixels).
3. **Fragment shading** (texturing, lighting) → ** framebuffer** (with depth/stencil tests, blending).

**2D** pipelines simplify this: transform paths → rasterize → blend to target buffer.

### 7) 2D fundamentals
- **Lines**: explicit line equations; **Bresenham** for integer-only rasterization.
- **Shapes**: rectangles, polygons; **fill rules** (even-odd vs non-zero winding).
- **Transformations**: translate, rotate, scale, shear — compose as **matrices** (homogeneous coordinates).

### 8) 3D basics
- **Coordinates**: right-handed vs left-handed systems; column vs row vectors (stay consistent).
- **Transformations**: model matrix (object → world), view (world → camera), projection (camera → clip).
- **Orthographic**: parallel sizes preserved (engineering/CAD, some UI).
- **Perspective**: foreshortening — distant objects smaller (realistic 3D).

### 9) Double buffering & VSync
- **Single buffer**: drawing while scanning out can cause **tears** (part old, part new image).
- **Double buffering**: render to **back buffer**, **swap** to front at a safe time.
- **VSync**: swap aligned to **vertical blanking** to reduce tearing; can add latency if paced poorly (triple buffering, adaptive sync technologies exist on modern stacks).

### 10) Multimedia + graphics

| | **Graphics (typical)** | **Video (typical)** |
|--|------------------------|---------------------|
| Source | Procedural/vector/UI layers | **Compressed** bitstream (file/stream) |
| Memory | Often RGB/BGRA buffers | **YUV** surfaces post-decode |
| Time | Event-driven + animation timeline | **Fixed frame rate** cadence |

**How video frames are rendered (simplified)**:
1. **Demux** container (MP4, etc.) → compressed packets.
2. **Decode** → **YUV** frame(s) in memory (e.g. NV12).
3. **Color convert / scale** if needed → RGB/RGBA for the compositor or GL texture.
4. **Present** via display stack (DRM plane, Wayland buffer, or blit to framebuffer).

**Pixel formats**
- **RGB888 / ARGB8888**: packed RGB (+ optional alpha).
- **YUV420**: chroma **half** resolution H and V vs luma (multiple plane layouts).
- **NV12**: Y plane + **interleaved UV** (very common outputs from decoders/hardware).

**Scaling / rotation / CSC**
- **Nearest / bilinear / bicubic** scaling — quality vs cost.
- **Rotation** often via matrix warp or 90° tricks (stride swaps).
- **Color conversion** (YUV→RGB) is **per-pixel math**; SIMD or GPU fragment shaders speed this up.

---

## === LINUX & EMBEDDED GRAPHICS ===

### Linux graphics stack (conceptual bottom → top)
1. **Hardware**: GPU, display controller (SoC), panel/bridge (DSI/HDMI/LVDS).
2. **Kernel**: **DRM (Direct Rendering Manager)** + **KMS (Kernel Mode Setting)**; drivers expose **devices** and **ioctl** interfaces.
3. **Userspace**: **libdrm** (GLM/low-level), **Mesa** (OpenGL/Vulkan drivers), compositors (**Wayland**) or legacy **X11**, toolkits (Qt/GTK).

### `/dev/fb0` (legacy framebuffer)
- Historically: mmap framebuffer memory, draw with CPU.
- Still seen on **simple embedded** kernels; **limited** for modern GPUs (no vsync/planes story in fbdev alone).
- Today prefer **DRM** for mode-setting and zero-copy paths where possible.

### DRM/KMS (what to learn)
- **Encoder / Connector / CRTC / Plane** objects.
- **Atomic commits**: bundle mode + plane configuration updates.
- **GEM/TTM** buffer management (big topic); practical start: **gbm** + EGL for GLES.

### Wayland vs X11
- **X11**: server-centric; many round trips; legacy but still deployed.
- **Wayland**: clients use **EGL + dmabuf** (commonly) and talk protocol to **compositor**; aligns better with modern GPUs and ** tearing reduction** at compositor level.

### GPU vs CPU rendering
- **CPU**: flexible, easy for small panels / bring-up; bottlenecks on fill rate and scaling.
- **GPU**: parallel **fragment** work; handles 3D, large blits, shaders, often **display overlays** (hardware planes).

### Embedded graphics specifics

**SPI display (ILI9341 / ST7789)**
- **MCU or Linux** drives a **serial bus**; limited **Mbps** vs parallel RGB or DSI.
- Partial window updates, **dirty rectangles** to save bandwidth.
- Often **single full-frame buffer** in RAM or **double buffer** if RAM allows.

**Framebuffer rendering on MCU/Linux**
- Same idea: back buffer → flush to panel; SPI means **chunked** transfers.

**Performance constraints**
- **Memory bandwidth**, **CPU time**, **bus width**, **SPI clock**, **IRQ latency**.
- **Color depth tradeoffs** (RGB565 vs RGB888).

**DMA usage**
- **DMA** offloads memory copies (e.g. SPI TX FIFO fill from RAM).
- Watch for: **alignment**, **cache lines** (may need flush/invalidate on DMA buffers), **coherency** between CPU writes and DMA reads.

---

## === HANDS-ON PROJECTS ===

### Beginner
1. **Framebuffer pixels**: map `/dev/fb0` (where available); fill screen with solid color; plot single pixels from a `put_pixel(x,y,color)` helper (respect **stride** and **endianness**/offsets from `fb_var_screeninfo` / `fb_fix_screeninfo`).
2. **Lines & rectangles**: implement **Bresenham**; draw a simple **UI mock** (buttons as rects).
3. **PPM loader**: read **PPM** (easy format) and blit to framebuffer; then **BMP** or raw RGB.

### Intermediate
4. **Image viewer**: zoom (nearest-neighbor first), pan; handle **stride** and **formats**.
5. **ASCII video preview**: decode with **FFmpeg** to small resolution; convert a few rows to ASCII (sanity-check pipeline) before raw RGB work.
6. **Video player (software path)**: FFmpeg decode → **scale + YUV→RGB** (e.g. **libswscale**) → **SDL2** texture or mmap’d buffer if on fbdev test setup.

### Advanced / embedded-oriented
7. **DRM modeset “smoke test”**: use **libdrm** examples mindset: list connectors, set mode (careful on real hardware).
8. **OpenGL ES triangle + texture**: **EGL** + **GMU/Mesa** on a board; render decoded video frame as texture (upload path matters).
9. **SPI display bring-up**: Linux **spidev** or kernel driver; send **init sequence**; `fb_shim` or userspace blit loop; measure **FPS** vs SPI clock and update strategy.

**Safety note:** Mode-setting on real monitors can flicker; test on disposable panels first. Prefer VMs/development boards with serial console.

### C program sketches (patterns)

**`put_pixel` with mmap (fbdev-style, illustrative — verify `fb` structs on your system):**
```c
/* Pseudocode pattern: always use fix.line_length for stride */
uint8_t *base = mmap(NULL, screensize, PROT_READ|PROT_WRITE, MAP_SHARED, fbfd, 0);
void putpixel(uint8_t *fb, int x, int y, int stride, uint32_t rgba) {
    uint32_t *p = (uint32_t *)(fb + y * stride + x * 4);
    *p = rgba;
}
```

**Line drawing**: implement Bresenham for integer endpoints; branchless variants optional later.

**Images**: decode externally or load uncompressed first; **respect stride** when copying rows (`memcpy` per row if pitch differs).

**OpenGL ES intro path**
- Context: **EGL** (`eglGetDisplay`, `eglCreateContext`, surface from **gbm** or native window).
- API subset: **VAOs/VBOs**, **vertex/fragment shaders** GLSL ES, **textures**, basic MVP matrices.

---

## === STUDY MATERIALS ===

### Official / documentation (high signal)
- **Linux DRM/KMS**: [https://dri.freedesktop.org/docs/drm/](https://dri.freedesktop.org/docs/drm/)
- **Wayland**: [https://wayland.freedesktop.org/](https://wayland.freedesktop.org/)
- **Mesa / Gallium** (context): [https://mesa3d.org/](https://mesa3d.org/)
- **Khronos OpenGL ES**: [https://www.khronos.org/opengles/](https://www.khronos.org/opengles/) — specs + **OpenGL ES Shading Language**
- **FFmpeg documentation**: [https://ffmpeg.org/documentation.html](https://ffmpeg.org/documentation.html) (libavformat, libavcodec, libswscale)
- **Cairo**: [https://www.cairographics.org/](https://www.cairographics.org/) — 2D vector rendering
- **SDL**: [https://wiki.libsdl.org/SDL3/FrontPage](https://wiki.libsdl.org/SDL3/FrontPage) (check version; SDL2 still widespread)

### Books
- **“Computer Graphics: Principles and Practice”** (Hughes et al.) — broad classic (heavy).
- **“Fundamentals of Computer Graphics”** (Steve Marschner, Peter Shirley) — modern readable survey.
- **“Real-Time Rendering”** (Akenine-Möller et al.) — real-time 3D deep dive (advanced).
- **“Linux Device Drivers”** (Corbet, Rubini, Hartman) — understanding kernel interfaces around DRM (selected chapters + DRM docs).

### Online courses / references
- **Scratchapixel** (graphics pipeline, math): [https://www.scratchapixel.com/](https://www.scratchapixel.com/)
- **Learn OpenGL** (OpenGL; concepts transfer to GLES with API differences): [https://learnopengl.com/](https://learnopengl.com/)

### YouTube / channels (search these names; pick recent playlists)
- **Benjamin Taft** (embedded Linux topics vary; search DRM/embedded graphics).
- **Jacob Sorber** (C, systems snippets — supplemental).
- **Vulkanised / GPUOpen / Khronos** (talks on modern graphics — more advanced).

### Community
- **freedesktop** mailing lists / IRC/Matrix for Wayland/Mesa ecosystems.
- **Kernel newbies** resources if you touch driver edges.

---

## === INTERVIEW QUESTIONS ===

### Foundations
- **Raster vs vector?** When would you choose each?
- **What is stride and why can it differ from `width * bpp`?**
- **Explain RGB vs YUV and why H.264 often uses 4:2:0.**
- **What causes screen tearing?** How do double buffering and VSync help?

### Linux / embedded systems
- **Difference between fbdev and DRM/KMS?**
- **What is a CRTC and a plane in KMS?**
- **Wayland vs X11 at a high level?**
- **GPU vs CPU blit: tradeoffs on an SoC?**
- **Why is NV12 common from decoders?** How would you render it on screen?

### Multimedia
- **Demux vs decode vs parse** — clarify pipeline stages.
- **How would you implement a simple video viewer with FFmpeg + SDL?** Where are the bottlenecks?
- **Nearest vs bilinear scaling — artifacts and cost?**

### Embedded / SPI displays
- **Why is SPI FPS limited?** What strategies improve perceived performance?
- **When is DMA important for display output?** What can go wrong with caches?

### Real-world scenarios
- **Green screen / overlay**: you have decoded YUV and an RGB UI — how do you composite efficiently?
- **Rotation**: user rotates device 90° — what layers must know (stride, transform, sensor vs software rotation)?
- **New board bring-up**: no graphics yet — what’s your path from **boot logo** to **userspace Qt**?

### Concepts to emphasize for “system-level” roles
- **Buffer lifetime** and **zero-copy** (`dmabuf`, EGLImages).
- **Color spaces** (BT.601 vs BT.709 vs BT.2020) at least at naming level.
- **Sync** (implicit/explicit fences on modern stacks — optional advanced).
- **Atomic modesetting** failure modes and debugging (`modetest`, `drm_info` tools ecosystem).

---

## Appendix — Tool roles (quick reference)

| Tool / API | Role |
|------------|------|
| **OpenGL / GLES** | GPU-accelerated 2D/3D; shaders; textures for video frames. |
| **SDL** | Cross-platform window/events; textures/renderers; good for prototypes. |
| **FFmpeg** | Reading media; decoding to raw frames; timestamps; rescale/CSC via swscale. |
| **Cairo** | 2D vector graphics, PDF-like quality when rasterized; used in toolkits indirectly. |

---

*End of roadmap. Add board-specific notes (SoC, kernel version, driver names) in a separate `BOARD_NOTES.md` as you progress.*
