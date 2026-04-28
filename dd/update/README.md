# Audio playback + ILI9225 SPI LCD (user space)

This folder adds **concurrent** full-screen RGB565 updates to `/dev/ili9225_char` while **aplay** runs in a separate process. Decode stays in user space (ffmpeg); the ALSA PCM kernel driver is unchanged.

---

## === ARCHITECTURE ===

**Components**

| Layer | Role |
|--------|------|
| **ffmpeg** | Decodes MP3/AAC/FLAC → raw S16_LE stereo @ 44.1 kHz (file) |
| **aplay** (child process) | Streams PCM to ALSA (`hw:X,Y`) — **blocking I/O in the child only** |
| **Parent / pthread** | Opens `/dev/ili9225_char`, `write()` full frames (176×220×2 bytes) |
| **ili9225 kernel module** | `ili_write` → `drawImage(0,0,176,220, …)` — no change to ALSA |

**Synchronization (no sample-accurate A/V lock)**

- **Process model:** `fork` + `exec(aplay)` returns immediately; the parent does **not** block on audio.
- **Display thread:** A dedicated **pthread** waits on audio completion via `waitpid(aplay_pid, …)` while either:
  - pushing **one** static frame after decode, or
  - looping **animation** frames with `waitpid(..., WNOHANG)` + `nanosleep` for rough FPS.
- **Coupling:** LCD updates are **best-effort** vs. audio time. For a demo/interview, that is enough; sample-accurate sync would need timestamps (e.g. from decoded audio position or a timer tied to known duration).

**Why not put LCD in the aplay process?**

- Keeps **one** writer to `/dev/ili9225_char` and avoids coupling display logic to ALSA’s blocking `write` loop.
- **Interview story:** clear separation — audio pipeline vs. SPI framebuffer path.

---

## === UPDATED USER SPACE CODE (main.c) ===

Delivered as `update/main.c` (standalone; does not overwrite `dd/main.c`).

**Behaviour**

- Interactive prompt for audio (same discovery rules as `audio_format_player_interactive`).
- Optional CLI:
  - `--lcd PATH` — default `/dev/ili9225_char` or `LCD_DEV`
  - `--image FILE.raw` — single full-screen RGB565 frame
  - `--anim-dir DIR` — all `*.raw` in `DIR`, sorted by name, cycled at `--fps`
  - `--sync-aplay` — old behaviour: blocking aplay, no LCD concurrency

**Build**

```bash
cd update && make
```

Binary: `audio_lcd_player` (links **-pthread**).

---

## === IMAGE PREPARATION STEPS ===

**Framebuffer format (must match driver)**

- Resolution: **176×220** (see `drawImage` in your `ili9225_spi_driver.c`).
- Pixel format: **RGB565 little-endian** (`uint16_t` per pixel, `rgb565le`).
- **One frame size:** `176 * 220 * 2 = 77440` bytes.

**Important:** The driver’s `write()` path allocates `len` bytes and calls `drawImage(..., width*height, …)` with fixed 176×220. Always send a **full** 77440-byte write so kernel and userspace agree (avoid partial-buffer hazards).

**ffmpeg (recommended — explicit format)**

Scale/pad to exact panel size, then raw RGB565 LE:

```bash
ffmpeg -y -i input.png -vf "scale=176:220:force_original_aspect_ratio=decrease,pad=176:220:(ow-iw)/2:(oh-ih)/2:color=black" \
  -pix_fmt rgb565le -f rawvideo frame.raw
```

**ImageMagick (`convert`)**

```bash
convert input.png -resize 176x220! -depth 16 RGB565:frame.raw
```

Verify size:

```bash
wc -c frame.raw   # expect 77440
```

**Many frames (animation)**

Option A — extract from video:

```bash
mkdir -p anim && ffmpeg -y -i clip.mp4 -vf "fps=10,scale=176:220:force_original_aspect_ratio=decrease,pad=176:220:(ow-iw)/2:(oh-ih)/2:color=black" \
  -pix_fmt rgb565le -f image2 anim/%03d.raw
```

Option B — numbered PNGs:

```bash
for f in *.png; do
  ffmpeg -y -i "$f" -vf "scale=176:220:force_original_aspect_ratio=decrease,pad=176:220:(ow-iw)/2:(oh-ih)/2:color=black" \
    -pix_fmt rgb565le -f rawvideo "anim/${f%.png}.raw"
done
```

Sort order: name sort (`frame_002.raw` before `frame_010.raw` if zero-padded).

---

## === MULTI-FRAME DISPLAY (OPTIONAL) ===

The program cycles files in `--anim-dir` (sorted `*.raw`) at `--fps` while polling whether **aplay** has exited.

**Rough sync with audio**

- **Time-based:** pick `--fps` so total animation loops feel acceptable for the clip length (heuristic).
- **Better (not implemented):** decode duration with `ffprobe`, set FPS = `n_frames / duration_seconds`.
- **Advanced:** single process using **libav** + **libasound** + LCD writes on a timeline — heavier; kept out of this “simple demo” tree.

---

## === EXECUTION STEPS ===

### 1. Kernel: ALSA PCM + ILI9225

- Build/install your **ALSA platform PCM** driver (as in your tree) and ensure `aplay -l` shows the card.
- Build **ILI9225 SPI** module from your `ili9225_spi_driver.c` (Device Tree binding `compatible = "ilitek,ili9225"`).
- **Load order:** SPI/display as needed, then ALSA — order usually does not matter for this split.

```bash
# example — adjust paths/module names
insmod ili9225_spi.ko   # or modprobe from install path
# insmod your_alsa_pcm.ko
```

### 2. Device nodes

```bash
ls -l /dev/ili9225_char
# ALSA: hw:0,0 or as enumerated — match with aplay -l
```

### 3. User program

```bash
cd /path/to/dd/update
make
```

### 4. Run

**Audio only (same as before, async aplay, no LCD):**

```bash
./audio_lcd_player hw:0,0
```

**Audio + static image:**

```bash
./audio_lcd_player hw:0,0 --lcd /dev/ili9225_char --image ./splash.raw
```

**Audio + animation:**

```bash
./audio_lcd_player hw:0,0 --anim-dir ./anim --fps 12
```

**Environment overrides:**

```bash
export AUDIO_HW=hw:1,0
export LCD_DEV=/dev/ili9225_char
./audio_lcd_player --image ./splash.raw
```

---

## Kernel driver note

Your `ili_write` handler should receive **exactly** one full frame (77440 bytes) per `write()` for a clean mapping to `drawImage(0,0,176,220,...)`. If you ever extend the driver, consider rejecting `len != 176*220*2` with `-EINVAL` to catch userspace mistakes early.

---

## Files in `update/`

| File | Purpose |
|------|---------|
| `main.c` | ffmpeg decode, async aplay, LCD `write()`, optional pthread display |
| `Makefile` | `make` → `audio_lcd_player` |
| `README.md` | This document |

The SPI driver source you pasted is **unchanged** here; only user space and documentation live under `update/`.
