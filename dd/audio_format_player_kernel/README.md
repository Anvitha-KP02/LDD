## audio_format_player_kernel

Optional kernel-side companion module that exposes the **selected audio format**.

### Important

- **No decoding in kernel** (no MP3/AAC/FLAC decode)
- This module only stores/exposes selection via:
  - `/dev/audio_format_ctrl`
  - `/sys/class/misc/audio_format_ctrl/format`

### Build / load

```bash
make
sudo insmod audio_format_ctrl.ko
```

### Use

```bash
# write format
echo aac | sudo tee /sys/class/misc/audio_format_ctrl/format

# or numeric
echo 3 | sudo tee /sys/class/misc/audio_format_ctrl/format   # flac

# read format
cat /sys/class/misc/audio_format_ctrl/format
cat /dev/audio_format_ctrl
```

This is purely a control/visibility interface; playback is still:
user space `ffmpeg` -> RAW -> `aplay -D hw:X,Y` -> ALSA PCM driver.

