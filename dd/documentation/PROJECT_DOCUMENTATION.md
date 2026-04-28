# Project Documentation

## 1. Overview

This project documents and integrates two components:

- `audio_format_player/main.c` (user-space player)
- `audio_format_player_kernel/audio_format_ctrl.c` (kernel format control module)

The design keeps codec processing in user space and only simple control/state in kernel space.

## 2. Detailed Documentation: `audio_format_player/main.c`

### 2.1 Objective

Provide interactive playback for `.mp3`, `.aac`, and `.flac` files by:

1. identifying the file from a user query,
2. converting to fixed RAW PCM with `ffmpeg`,
3. playing the RAW stream via ALSA `aplay`.

### 2.2 Constants and Defaults

- `PCM_RATE_HZ = 44100`
- `PCM_CHANNELS = 2`
- fixed sample format for playback: `S16_LE`
- default ALSA device: `hw:0,0`
- override priority:
  - CLI argument (`argv[1]`)
  - environment `AUDIO_HW`
  - hardcoded default

### 2.3 Function-Level Behavior

- `trim_newline(char *s)`
  - removes trailing newline from user input.

- `file_exists_regular(const char *path)`
  - validates that a path exists and is a regular file.

- `run_execvp(char *const argv[])`
  - forks child process, runs command with `execvp`, waits with `waitpid`.
  - returns error for abnormal exit or non-zero status.

- `convert_to_raw_pcm(const char *input_path, const char *output_raw_path)`
  - builds and executes `ffmpeg` command:
    - input: compressed audio
    - output: `s16le`, `pcm_s16le`, `2ch`, `44100Hz`.

- `play_raw_via_aplay(const char *hw_dev, const char *raw_path)`
  - builds and executes `aplay` command with RAW format parameters.

- `str_endswith_case(const char *s, const char *suffix)`
  - case-insensitive suffix check utility.

- `detect_format_from_extension(const char *path)`
  - maps extension to format id:
    - `.mp3` -> `1`
    - `.aac` -> `2`
    - `.flac` -> `3`.

- `format_to_string(int sel)`
  - format id to printable name: `MP3`, `AAC`, `FLAC`.

- `is_supported_audio_file(const char *name)`
  - true for the three supported extensions.

- `build_exact_basename_candidate(...)`
  - creates `basename + extension` candidate safely.

- `find_audio_file(const char *query, char *out_path, size_t out_sz)`
  - file matching priority:
    1. direct supported file path
    2. basename with extension probing
    3. extension query (like `.mp3`)
    4. substring match in current directory for supported files

### 2.4 Main Execution Flow

1. Resolve ALSA device from argument/environment/default.
2. Print startup banner with PCM contract.
3. Read user query from stdin.
4. Validate input not empty.
5. Find matching file via `find_audio_file`.
6. Detect format and validate support.
7. Create temporary RAW file using `mkstemp`.
8. Convert with `convert_to_raw_pcm`.
9. Play with `play_raw_via_aplay`.
10. Remove temp file (`unlink`) and exit.

### 2.5 Error Handling

- rejects empty input
- reports when no matching file is found
- rejects unsupported extension
- handles command failures (`ffmpeg` / `aplay`) via exit status
- ensures temporary file cleanup on failure and success

## 3. Detailed Documentation: `audio_format_player_kernel/audio_format_ctrl.c`

### 3.1 Objective

Expose selected audio format state through kernel interfaces for visibility/control only.

No decoding and no PCM streaming logic exist in this module.

### 3.2 Core Data and Synchronization

- enum `audio_fmt`:
  - `AUDIO_FMT_MP3 = 1`
  - `AUDIO_FMT_AAC = 2`
  - `AUDIO_FMT_FLAC = 3`
- global state: `current_fmt` (default MP3)
- synchronization: `DEFINE_MUTEX(fmt_lock)`

### 3.3 Conversion Helpers

- `fmt_to_str(int fmt)`
  - enum -> `"mp3" | "aac" | "flac" | "unknown"`.

- `str_to_fmt(const char *s)`
  - accepts text (`mp3/aac/flac`) or numbers (`1/2/3`)
  - uses `sysfs_streq`
  - returns `-EINVAL` on invalid input

### 3.4 Character Device Interface

Device path:

- `/dev/audio_format_ctrl`

Handlers:

- `afc_read(...)`
  - reads `current_fmt` under mutex
  - returns textual format with newline via `simple_read_from_buffer`.

- `afc_write(...)`
  - validates length and user copy
  - parses input with `str_to_fmt`
  - updates state under mutex
  - returns written length

File operations include:

- `.owner = THIS_MODULE`
- `.read = afc_read`
- `.write = afc_write`
- `.llseek = no_llseek`

### 3.5 Sysfs Interface

Path:

- `/sys/class/misc/audio_format_ctrl/format`

Handlers:

- `format_show(...)` -> returns current format string
- `format_store(...)` -> parses and sets current format

Declared as:

- `DEVICE_ATTR_RW(format)`

### 3.6 Module Init/Exit

- `afc_init()`
  - registers misc device (`misc_register`)
  - creates sysfs file (`device_create_file`)
  - prints informative log lines

- `afc_exit()`
  - removes sysfs file (`device_remove_file`)
  - deregisters misc device (`misc_deregister`)

## 4. End-to-End Working

1. User loads kernel module.
2. Control endpoints become visible.
3. User runs player and enters query.
4. Player finds audio file and identifies format.
5. Player converts compressed data to RAW PCM through `ffmpeg`.
6. Player plays RAW PCM through ALSA using `aplay`.
7. Kernel module can be read/written independently to track selected format state.

## 5. Build and Execution Steps

```bash
cd dd/audio_format_player_kernel
make
sudo insmod audio_format_ctrl.ko
ls -l /dev/audio_format_ctrl
cat /sys/class/misc/audio_format_ctrl/format
```

```bash
cd ../audio_format_player
make
./audio_format_player
```

Optional:

```bash
./audio_format_player hw:1,0
AUDIO_HW=hw:1,0 ./audio_format_player
```

Kernel control test:

```bash
echo aac | sudo tee /sys/class/misc/audio_format_ctrl/format
cat /sys/class/misc/audio_format_ctrl/format
cat /dev/audio_format_ctrl
```

Unload:

```bash
cd ../audio_format_player_kernel
sudo rmmod audio_format_ctrl
```

## 6. Limitations and Notes

- player search is only in current directory (non-recursive)
- playback path expects fixed PCM contract (`S16_LE`, `2ch`, `44100Hz`)
- kernel module does not enforce or synchronize with player selection automatically
- kernel module is optional for playback pipeline

## 7. Reference Files

- `audio_format_player/main.c`
- `audio_format_player_kernel/audio_format_ctrl.c`
- `documentation/FLOWCHART.md`
- `documentation/README.md`
