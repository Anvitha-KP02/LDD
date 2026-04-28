# Audio Format Player Documentation

This documentation describes these two programs:

- User-space app: `audio_format_player/main.c`
- Kernel module: `audio_format_player_kernel/audio_format_ctrl.c`

The playback architecture is intentionally split:

- decode and playback orchestration in user space (`ffmpeg` + `aplay`)
- lightweight format-control state in kernel space (misc device + sysfs)

## Program 1: `audio_format_player/main.c`

### Purpose

`main.c` is an interactive CLI player that accepts a user query, resolves an audio file (`.mp3`, `.aac`, `.flac`), converts it to fixed RAW PCM, and plays it through ALSA.

### What It Handles

- input capture from stdin
- smart file lookup in current directory
- format detection by extension (case-insensitive)
- conversion command execution via `ffmpeg`
- playback command execution via `aplay`
- temporary file lifecycle in `/tmp`
- ALSA device selection from:
  1) argv (`./audio_format_player hw:1,0`)
  2) env (`AUDIO_HW`)
  3) default (`hw:0,0`)

### Runtime Pipeline

1. Read user query (`fgets` + newline trim).
2. Resolve input file:
   - exact path if valid file with supported extension
   - basename + auto extension probing (`.mp3/.aac/.flac`)
   - extension query (example: `.mp3`)
   - substring query against supported files
3. Detect format (`MP3` / `AAC` / `FLAC`) from extension.
4. Create temporary output file via `mkstemp("/tmp/decoded_XXXXXX")`.
5. Run `ffmpeg` to decode into:
   - signed 16-bit little-endian (`s16le`)
   - stereo (`2ch`)
   - 44.1 kHz (`44100`)
6. Run `aplay` with matching RAW parameters.
7. Delete temporary file (`unlink`) on both success and failure paths.

### Command Shapes Used

```bash
ffmpeg -y -hide_banner -loglevel error \
  -i <input_audio> \
  -f s16le -acodec pcm_s16le -ac 2 -ar 44100 \
  <tmp_raw_file>
```

```bash
aplay -D <hw:X,Y> -t raw -f S16_LE -c 2 -r 44100 <tmp_raw_file>
```

## Program 2: `audio_format_player_kernel/audio_format_ctrl.c`

### Purpose

`audio_format_ctrl.c` is a kernel control module that stores and exposes the selected format. It does not decode audio.

### What It Exposes

- misc character device: `/dev/audio_format_ctrl`
- sysfs attribute: `/sys/class/misc/audio_format_ctrl/format`

### Supported Format Inputs

- text: `mp3`, `aac`, `flac`
- numeric: `1`, `2`, `3`

### Internal Behavior

- keeps current format in `current_fmt`
- guards shared state using `fmt_lock` mutex
- maps values through:
  - `fmt_to_str()` for read output
  - `str_to_fmt()` for write parsing
- char-device handlers:
  - `afc_read()` -> returns `<format>\n`
  - `afc_write()` -> validates and updates format
- sysfs handlers:
  - `format_show()` -> returns `<format>\n`
  - `format_store()` -> parses and updates format

### Module Lifecycle

- `afc_init()`:
  - registers misc device
  - creates sysfs file
- `afc_exit()`:
  - removes sysfs file
  - deregisters misc device

## Build and Run

### Build

```bash
cd dd/audio_format_player_kernel
make
cd ../audio_format_player
make
```

### Run

```bash
cd dd/audio_format_player_kernel
sudo insmod audio_format_ctrl.ko
ls -l /dev/audio_format_ctrl
cat /sys/class/misc/audio_format_ctrl/format

cd ../audio_format_player
./audio_format_player
```

Optional hardware override:

```bash
./audio_format_player hw:1,0
AUDIO_HW=hw:1,0 ./audio_format_player
```

Unload module:

```bash
cd ../audio_format_player_kernel
sudo rmmod audio_format_ctrl
```

## Quick Validation Commands

```bash
echo aac | sudo tee /sys/class/misc/audio_format_ctrl/format
cat /sys/class/misc/audio_format_ctrl/format
cat /dev/audio_format_ctrl
```

## Related Documentation

- Detailed design and function-level notes: `documentation/PROJECT_DOCUMENTATION.md`
- Detailed flowcharts for both programs: `documentation/FLOWCHART.md`
