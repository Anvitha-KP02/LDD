# v4l2_rpi_display — C + FFmpeg userspace video on SPI framebuffer

All sections below match the requested design. **No kernel video decode.** Only **C**; decoding uses **libavformat**, **libavcodec**, and **libswscale**.

---

## === SYSTEM ARCHITECTURE ===

```
┌────────────────────────────────────────────────────────────────────┐
│ USER SPACE (video_player.c)                                        │
│   Menu CLI → file pick (basename or extension)                      │
│   libavformat: demux (MP4/MKV/AVI/WEBM containers)                  │
│   libavcodec:  decode video → AVFrame (e.g. YUV420P)                │
│   libswscale:  scale + convert → AV_PIX_FMT_RGB565LE               │
│   Double-buffered AVFrames (rgb_a / rgb_b) → mmap(/dev/fbN) blit    │
└────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌────────────────────────────────────────────────────────────────────┐
│ KERNEL                                                             │
│   fbtft / SPI framebuffer driver: exposes linear FB, RGB565 mode     │
│   Driver does NOT decode video — only scans out raw pixel memory     │
└────────────────────────────────────────────────────────────────────┘
```

**Pipeline:** User input → File selection → FFmpeg decode (userspace) → RGB565 conversion (swscale) → SPI display (kernel FB driver).

---

## === USER SPACE C PROGRAM (COMPLETE CODE) ===

Single translation unit: **`video_player.c`** (menu, glob/scandir-style search, libav, mmap blit).

Build output: **`video_player`** (see **Makefile**).

---

## === FILE SELECTION LOGIC (NAME vs FORMAT) ===

| Input | Meaning | Behavior |
|--------|---------|----------|
| `clip` | Base name | Looks for `clip.mp4`, `clip.mkv`, `clip.avi`, `clip.webm` in `-d` directory. 0 matches → error; 2+ → numbered menu. |
| `.mp4` or `mp4` | Format | Globs `dir/*.{ext}` for supported types, sorted list, numbered menu (cancel with `0`). |

Optional non-interactive: `./video_player -W 240 -H 320 /path/to/file.mp4`

---

## === FFmpeg API USAGE ===

1. **`avformat_open_input`**, **`avformat_find_stream_info`** — open container.
2. **`av_find_best_stream(..., AVMEDIA_TYPE_VIDEO, ...)`** — pick video stream.
3. **`avcodec_alloc_context3`**, **`avcodec_parameters_to_context`**, **`avcodec_open2`** — decoder.
4. Read loop: **`av_read_frame`** → **`avcodec_send_packet`** → **`avcodec_receive_frame`** until `AVERROR(EAGAIN)` / EOF.
5. Flush: **`avcodec_send_packet(NULL, NULL)`** then drain **`avcodec_receive_frame`**.
6. **`av_frame_alloc`**, **`av_frame_get_buffer`** for decoded frame and two RGB565 target frames.
7. **`sws_getContext`** / **`sws_scale`** — resize to display WxH, output **`AV_PIX_FMT_RGB565LE`**.
8. Teardown: **`sws_freeContext`**, **`av_frame_free`**, **`av_packet_free`**, **`avcodec_free_context`**, **`avformat_close_input`**.

---

## === RGB565 CONVERSION ===

`sws_scale` produces **little-endian RGB565** (`AV_PIX_FMT_RGB565LE`), 2 bytes/pixel, suitable for typical SPI TFT controllers and fbtft 16bpp modes.

---

## === SPI DISPLAY OUTPUT ===

- Opens `/dev/fb1` by default (`-f` override).
- **`FBIOGET_FSCREENINFO` / `FBIOGET_VSCREENINFO`** — line length and resolution.
- **`mmap`** framebuffer for efficient full-frame updates.
- **`fb_blit_rgb565`** copies each row: `min(xres,W) × min(yres,H)`, respecting **`finfo.line_length`** (stride may exceed `width * 2`).

Kernel SPI driver is assumed already loaded (overlay/module).

---

## === MAKEFILE ===

See **`Makefile`** in this directory:

- `pkg-config --cflags/libs` for **libavformat libavcodec libswscale libavutil**
- `make` → `./video_player`

---

## === EXECUTION STEPS (RPI) ===

```bash
sudo apt update
sudo apt install -y build-essential libavformat-dev libavcodec-dev \
    libswscale-dev libavutil-dev

cd /path/to/v4l2_rpi_display
make

# Ensure SPI framebuffer is active (e.g. fbtft) and mode matches -W/-H
sudo ./video_player -d "$HOME/Videos" -f /dev/fb1 -W 240 -H 320
# Or direct file:
./video_player -f /dev/fb1 -W 240 -H 320 ~/Videos/demo.mp4
```

User running the binary needs read/write access to the framebuffer (often `video` group or `sudo` for quick tests).

---

## === SAMPLE RUN ===

```text
$ ./video_player -d ~/Videos -f /dev/fb1 -W 240 -H 320

Enter video base name (e.g. video1) OR format (.mp4, .mkv, .avi, .webm):
> vacation

Framebuffer: /dev/fb1 (240x320, line_len=480 bytes)
Playing: /home/pi/Videos/vacation.mp4
Video: 1920x1080, codec=h264, target 240x320 @ 29.970 FPS
Playback finished (12400 frames).
```

```text
> .mkv

Matching files:
  [1] a.mkv
  [2] clip.mkv

Enter number to play (1-2), or 0 to cancel: 2
...
```

---

## === LIMITATIONS ===

- **Audio** is not decoded or played (video-only path).
- **Hardware-accelerated decode** (e.g. some DRM/V4L2 ffmpeg codecs) may yield **`AVFrame`** pixel formats **`libswscale` cannot consume directly** — use software decoder or extend with `av_hwframe_transfer_data` (not included here).
- **USB / SD throughput** and **software H.264** may limit FPS at high resolutions; pre-scale content or reduce bitrate for smooth playback.
- **Single visible buffer** on many SPI panels: double buffering is in **CPU memory**; the panel still shows updates as they are written — tearing may remain if the panel reads during write; full-frame blit minimizes flicker.

---

## License

`video_player.c`: same SPDX intent as project (MIT or GPL per your tree); architecture text: documentation only.
