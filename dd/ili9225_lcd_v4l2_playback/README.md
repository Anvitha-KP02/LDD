# ILI9225 SPI LCD — V4L2 output integration (Raspberry Pi 4)

This folder ties together **Device Tree**, the **out-of-tree V4L2 driver** in `../rpi_ili9225_v4l2`, and **userspace** playback (FFmpeg / GStreamer). The kernel module only accepts **raw RGB565** frames at **176×220**; all decoding and scaling happen in userspace.

## Layout

| Path | Purpose |
|------|--------|
| `overlay/ili9225-spi0-ce0-overlay.dts` | SPI0 CE0 + `ilitek,ili9225` + `dc-gpios` / `reset-gpios` / optional `backlight-gpios` |
| `scripts/play_to_lcd.py` | Interactive file pick + ffmpeg or GStreamer |
| `scripts/ffmpeg_examples.sh` | Copy-paste FFmpeg command |
| `scripts/gst_examples.sh` | Default GStreamer pipeline + commented variants |
| `play_to_lcd.c` | Minimal C wrapper that `exec`s ffmpeg |

## 1. Device tree overlay

Build and install on the Pi:

```bash
dtc -@ -I dts -O dtb -o ili9225-spi0-ce0.dtbo overlay/ili9225-spi0-ce0-overlay.dts
sudo cp ili9225-spi0-ce0.dtbo /boot/firmware/overlays/
```

In `/boot/firmware/config.txt`:

```text
dtparam=spi=on
dtoverlay=ili9225-spi0-ce0
```

Adjust **BCM GPIO numbers** in the DTS for your wiring (defaults: DC=25, RESET=24). SPI0: MOSI GPIO10, SCLK GPIO11, **CE0 GPIO8**.

- **`compatible`**: must be `ilitek,ili9225` (matches the driver).
- **`spi-max-frequency`**: start around **8–16 MHz**; increase after scope/logical checks if stable.
- **SPI mode**: ILI9225 typically **mode 0** (default). Add `spi-cpol;` / `spi-cpha` only if your board requires mode 3.
- **Optional `backlight-gpios`**: uncomment in the DTS; driver turns it **on** after successful registration and **off** on remove. Use `GPIOD_OUT_LOW` + active-high wiring, or set the flag cell to **1** (`GPIO_ACTIVE_LOW`) for active-low enable.

Avoid loading the **DRM `tiny ili9225`** (or any other driver) on the same SPI device.

## 2. Kernel module

From `../rpi_ili9225_v4l2`:

```bash
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
sudo insmod ili9225_v4l2.ko
```

Check:

```bash
dmesg | tail
v4l2-ctl --all -d /dev/video0
```

Driver behavior (recent improvements):

- **`spi_get_drvdata()`** / **`spi_set_drvdata()`** for instance data; **no global** device pointer.
- **DC GPIO**: looks up **`dc`** first, then **`rs`** (legacy).
- **RGB565 only**: **`VIDIOC_S_FMT`** rejects formats other than `V4L2_PIX_FMT_RGB565`; **`G_FMT` / `TRY_FMT`** fixed 176×220.
- **SPI**: one **GRAM burst** per frame (`ili9225_wr_gram`), chunked by `spi_max_transfer_size`.

## 3. Userspace playback

### FFmpeg (generic: MP4, MKV, AVI, WEBM, …)

```bash
ffmpeg -re -i your_video.mp4 \
  -vf "scale=176:220:flags=fast_bilinear" \
  -pix_fmt rgb565le \
  -f v4l2 /dev/video0
```

- Omit **`-re`** to push frames as fast as possible (stress test).
- If colors are wrong, try property **`swap-bytes`** in the overlay or adjust pixel format after reading the panel data sheet / scope.

### GStreamer

```bash
gst-launch-1.0 -e uridecodebin uri=file:///path/to/video.webm name=d \
  d. ! queue ! videoconvert ! videoscale ! \
  video/x-raw,width=176,height=220,format=RGB16 ! \
  v4l2sink device=/dev/video0 sync=false
```

Install plugins on Debian/Raspberry Pi OS if needed: `gstreamer1.0-tools`, `gstreamer1.0-plugins-good`, `gstreamer1.0-libav`.

### Python helper

```bash
chmod +x scripts/play_to_lcd.py
./scripts/play_to_lcd.py -d /dev/video0 -i . -b ffmpeg
```

## 4. FPS / SPI performance tips

- **Raise `spi-max-frequency`** in small steps until artifacts appear, then back off.
- **Userspace**: use **two or more** buffered pipelines where possible (GStreamer queues; FFmpeg is often single-threaded on output but decode can use **`-threads`**).
- **Kernel**: the driver already pushes full frames in one GRAM transaction (plus command overhead); further gains are mostly **SPI clock** and **CPU load** for decode/scale.
- **GStreamer**: `queue max-size-buffers=2 leaky=downstream` can keep latency lower when the panel cannot sustain full FPS (drops frames instead of lagging).
- **Scheduling**: run playback on a lightly loaded CPU; isolate decode with `taskset` if needed.
- **Resolution**: always scale to **exactly 176×220** in userspace to match buffer size expectations.

## 5. Conflict with DRM ili9225

Blacklisting or disabling `CONFIG_TINYDRM_ILI9225` prevents a second driver from claiming the same SPI device. This V4L2 driver is **output-only** and does not decode video.
