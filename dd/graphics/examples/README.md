# Graphics examples

Small **Linux framebuffer** starters. Use on systems with `/dev/fb0` and appropriate permissions (often `video` group).

```bash
gcc -O2 -Wall -o fb_fill framebuffer_fill.c
./fb_fill
```

**Note:** `fb_var_screeninfo` offsets (`red.offset`, `green.offset`, …) vary by driver (RGB vs BGR). This code uses **32-bit XRGB** packing as a teaching default — adjust for your hardware.

See `../GRAPHICS_MULTIMEDIA_ROADMAP.md` for concepts and safer DRM/KMS paths for production.
