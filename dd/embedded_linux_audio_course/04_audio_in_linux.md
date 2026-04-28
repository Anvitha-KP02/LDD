=== 4. AUDIO IN LINUX ===

## Overview of ALSA (Advanced Linux Sound Architecture)
ALSA is Linux’s core audio subsystem. It provides:
- Kernel drivers and APIs for sound hardware
- A user-space library (`libasound`) that apps use to talk to kernel drivers
- Utilities (`aplay`, `arecord`, `amixer`, etc.)

In embedded systems, ALSA is the foundation. Higher-level stacks (PulseAudio/PipeWire) may exist on desktops, but many embedded products use ALSA directly.

Big picture:

```
User space                        Kernel space
----------                        ------------
App/Player/Recorder
   |
   v
libasound (ALSA userspace API) -> ALSA PCM core -> ALSA driver (ASoC/PCI/USB) -> HW
```

## Role of user space vs kernel space

### User space responsibilities
- Decode/compress (MP3/AAC/FLAC) unless a special HW offload is used
- Choose stream parameters: rate, channels, format
- Mix multiple streams (if you have a mixer daemon) or do it in-app
- Handle timing, buffering strategy, recovery from XRUN

### Kernel space responsibilities
- Provide standardized interfaces to audio hardware (PCM, controls, mixer elements)
- Manage DMA (often via DMAEngine)
- Program the audio controller (I2S/TDM/PDM) and codec via control buses (I2C/SPI)
- Provide interrupts and pointer updates for the ring buffer

Interview-ready line:
- **Kernel moves PCM reliably and configures hardware. User space creates/decodes the audio and feeds PCM.**

## What happens when audio is played (high-level)
Let’s say you run `aplay music.wav`.

1) **App parses file**
- WAV header gives format: rate/channels/bit depth

2) **ALSA device open**
- `snd_pcm_open()` opens something like `hw:0,0` or `default`

3) **Negotiation / hw params**
- App requests: `S16_LE`, 48kHz, stereo, etc.
- ALSA chooses a supported configuration (or uses plugins to convert)

4) **Buffer/period config**
- App sets buffer size and period size (or ALSA picks defaults)

5) **Prepare + start**
- `snd_pcm_prepare()`
- initial data written
- `snd_pcm_start()` or implicit start on first write

6) **Steady state streaming**
- App repeatedly writes PCM frames
- Kernel driver hands buffers to DMA
- DMA feeds I2S/TDM controller
- Codec converts digital → analog (DAC) → speaker output

7) **Interrupt cadence**
- On each “period” completion, hardware triggers interrupt
- Driver updates PCM “position” (pointer) and wakes user space if needed

Text flow with timing:

```
App write() --> [Kernel ring buffer] --> DMA --> I2S --> Codec DAC --> Speaker
                  ^ period IRQ every X frames ^
```

Why embedded teams care:
- Glitches usually come from timing/clock issues, underruns, or wrong hw_params.

