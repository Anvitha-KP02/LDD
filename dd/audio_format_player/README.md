## audio_format_player

User-space decode (FFmpeg) to **RAW PCM** then playback via **ALSA** into a kernel PCM driver.

### Key constraint (proper Linux architecture)

- Kernel driver: **RAW PCM only** (no MP3/AAC/FLAC decoding)
- User space: decode/compress handling via **FFmpeg**

### Build

```bash
make
```

### Run

```bash
# pick correct card,device shown by: aplay -l
./audio_format_player hw:1,0 /path/to/input.mp3

# or use env var
AUDIO_HW=hw:1,0 AUDIO_FILE=/path/to/input.mp3 ./audio_format_player
```

The program converts to `/tmp/decoded_XXXXXX` and then runs `aplay` with:

- format: `S16_LE`
- channels: `2`
- rate: `44100`
- type: `raw`

