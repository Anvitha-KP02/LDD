=== 6. AUDIO PIPELINE (VERY IMPORTANT) ===
Explain full flow:
MP3 → decoder → PCM → ALSA → driver → hardware

## The end-to-end picture

Most embedded Linux systems follow this exact separation of responsibilities:
- **Compressed formats** (MP3/AAC/FLAC) are handled in **user space**
- **Kernel** handles the **PCM** streaming to/from hardware

Text diagram (end-to-end):

```
   MP3 file/stream
        |
        v
  [Decoder in user space]
   (MP3 -> PCM samples)
        |
        v
  PCM frames (e.g., 48kHz, S16_LE, stereo)
        |
        v
  libasound (ALSA)
        |
        v
  ALSA kernel PCM core (ring buffer, timing)
        |
        v
  ALSA driver (ASoC PCM driver)
        |
        v
  DMA engine -> I2S/TDM controller -> Audio codec DAC -> Amplifier -> Speaker
```

## Step-by-step explanation (what actually happens)

### 1) MP3 bitstream is read
Your player reads MP3 bytes from:
- a file
- network stream
- flash partition

### 2) Decoder produces PCM
The decoder outputs PCM in a known format:
- sample rate (often 44.1k or 48k depending on content)
- channels (1 or 2 typically)
- sample format (often 16-bit or float internally)

Important:
- Decoder output may not match hardware “native” rate (e.g., 44.1k content on a 48k-only device).

### 3) (Optional) Resampling / Remix / Format conversion
If hardware requires different parameters:
- Resample 44.1k → 48k
- Convert float → S16_LE
- Downmix 2ch → 1ch or upmix 1ch → 2ch

Where this can happen:
- In the app itself
- In ALSA plugins (e.g., `plug`), but in embedded projects you often want explicit control for predictability

### 4) ALSA device configuration
App chooses a target PCM device:
- `hw:0,0` (direct hardware, no conversions)
- `plughw:0,0` (can do some conversions in ALSA plugin layer)
- `default` (may route through plugins/mixer daemons depending on system)

Interview tip:
- Use `hw:*` when debugging driver/hardware issues (removes “magic” conversions).

### 5) Streaming PCM into ALSA
App writes PCM frames via:
- `snd_pcm_writei()` (interleaved)
- or `mmap` mode for lower overhead

ALSA kernel:
- stores PCM in ring buffer
- tracks “available” space
- wakes the app based on period timing

### 6) Driver + DMA + hardware
Driver sets up:
- DMA buffers and DMA descriptors
- I2S/TDM registers (bit clock, word select, slot config)
- codec parameters (via DAI link in ASoC)

Then:
- DMA transfers memory → peripheral FIFO
- I2S shifts bits out at exact rate
- codec converts to analog

## Practical example pipeline on embedded Linux

Example: play MP3 via `ffmpeg` decoding to PCM and send to ALSA.

Conceptual:

```
ffmpeg (decode MP3 -> PCM) -> ALSA device
```

Example command (typical):
- Decode MP3 to WAV/PCM:
  - `ffmpeg -i input.mp3 -f wav out.wav`
- Or decode directly to ALSA (if available in your environment):
  - `ffmpeg -i input.mp3 -f alsa default`

If your product uses GStreamer:

```
filesrc ! mad/avdec_mp3 ! audioconvert ! audioresample ! alsasink
```

## Where glitches come from (real project mindset)
- **Underrun**: app/decoder can’t feed PCM fast enough (CPU spikes, SD-card latency)
- **Clock mismatch**: wrong PLL/DAI clocking causes speed/pitch artifacts
- **Wrong format**: S16 vs S32, endianness, channel order → noise or swapped channels
- **Rate conversion issues**: bad SRC quality or wrong assumptions about input rate

