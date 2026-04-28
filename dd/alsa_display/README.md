# Custom ALSA + SPI Display Integration

This folder contains:

- `kernel/custom_alsa_pcm.c`: custom ALSA PCM playback driver (kernel module)
- `kernel/Makefile`: build file for the kernel module
- `user/alsa_display_sync.c`: user-space audio + LCD multithread app
- `user/Makefile`: build file for user-space app

## 1) ALSA architecture (simple flow)

1. User app calls `snd_pcm_writei()` in ALSA library.
2. ALSA-lib sends frames to kernel PCM core using ioctl/write paths.
3. Kernel PCM core routes data to your driver `snd_pcm_ops`.
4. Your driver `.copy` stores data into runtime ring buffer.
5. Your driver timer simulates hardware consumption, updates `.pointer`, and calls `snd_pcm_period_elapsed()`.
6. ALSA core wakes user-space when more data can be written.

So the chain is:

`app -> libasound -> ALSA PCM core -> custom_alsa_pcm driver`

## 2) What this custom driver does

- Registers a new ALSA sound card with shortname `custom_alsa_card`
- Exposes one playback PCM device (`hw:X,0`)
- Supports fixed format:
  - `S16_LE`
  - stereo (`2` channels)
  - `44100 Hz`
- Implements key callbacks:
  - `.open`, `.close`
  - `.hw_params`, `.hw_free`
  - `.prepare`, `.trigger`, `.pointer`
  - `.copy` (used by `snd_pcm_writei` path)
- Uses software timer + circular buffer behavior to simulate DMA progress when hardware is absent.

## 3) Build steps

### Kernel module

```bash
cd kernel
make
```

### User app

Before building, place `image_array_hanuman.h` in `user/` (or adjust include path).

```bash
cd ../user
make
```

## 4) Load and verify ALSA card

```bash
sudo insmod kernel/custom_alsa_pcm.ko
aplay -l
```

Expected in `aplay -l`: a new card with name similar to `custom_alsa_card`.

To remove:

```bash
sudo rmmod custom_alsa_pcm
```

## 5) Run integration app

Use RAW PCM input (16-bit little-endian, stereo, 44.1kHz):

```bash
./user/alsa_display_sync /path/to/audio.raw hw:X,0 /dev/ili9225_char
```

Replace `X` with the card index shown by `aplay -l`.

## 6) Thread synchronization model

- Audio thread:
  - Opens `hw:X,0`
  - Streams PCM chunks with `snd_pcm_writei()`
  - On completion, sets `audio_done = 1`
- Image thread:
  - Opens `/dev/ili9225_char`
  - Loops through image frames and writes RGB565 data
  - Exits immediately when `audio_done` is set

Both threads run independently, so display updates do not block audio writes.
