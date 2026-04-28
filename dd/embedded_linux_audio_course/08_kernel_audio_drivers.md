=== 8. KERNEL AUDIO DRIVERS ===

This is the “driver engineer” view: what an ALSA PCM driver does, which callbacks matter, and how DMA/interrupt timing ties it all together.

## What an ALSA PCM driver does
An ALSA PCM driver provides a **PCM device** that supports:
- playback and/or capture
- specific hardware constraints (rates, formats, channels)
- DMA-based streaming with tight timing

From user space, it looks like a file descriptor you can write PCM frames to, but internally it is a timed pipeline:

```
ALSA core -> your PCM driver -> DMA -> audio peripheral (I2S/TDM) -> codec
```

## Key callbacks (open, hw_params, prepare, trigger, pointer)
In ASoC/ALSA PCM drivers, the standard lifecycle is:

### open()
Purpose:
- Allocate runtime resources
- Set constraints (supported rates/formats/channels)
- Initialize private state

Typical work:
- `snd_pcm_hw_constraint_*` calls
- set runtime->hw (capabilities)

### hw_params()
Purpose:
- Accept the chosen hardware parameters and allocate buffers if needed.

Typical work:
- Configure DMA buffer sizes
- Program clocking constraints (sometimes deferred)
- Select FIFO thresholds, word sizes, channels/slots

Key “interview line”:
- `hw_params()` is where the stream format becomes real hardware configuration.

### prepare()
Purpose:
- Put hardware into a ready-to-start state.

Typical work:
- Reset DMA pointers/FIFOs
- Program registers that depend on params
- Ensure codec DAI is configured (ASoC does a lot via DAPM/DAI ops)

### trigger(cmd)
Purpose:
- Start/stop/pause/resume the data flow.

Typical commands:
- `SNDRV_PCM_TRIGGER_START`
- `SNDRV_PCM_TRIGGER_STOP`
- `SNDRV_PCM_TRIGGER_PAUSE_PUSH/RELEASE`
- `SNDRV_PCM_TRIGGER_SUSPEND/RESUME`

Typical work:
- Start/stop DMA
- Enable/disable I2S peripheral
- Enable/disable interrupts

### pointer()
Purpose:
- Report current hardware playback/capture position in frames.

Why it matters:
- ALSA uses it for timing, `avail`, and XRUN detection.

Implementation approach:
- Read DMA engine residue or hardware position register
- Convert bytes → frames carefully

## Role of DMA
DMA is what makes continuous audio possible without burning CPU on per-sample I/O.

Why DMA:
- Audio requires precise, continuous throughput:
  - e.g., 48k frames/sec, each frame multiple bytes
- CPU jitter is too high to bit-bang audio reliably

Common DMA pattern:
- ALSA ring buffer in memory
- DMA cyclic transfer:
  - divides buffer into periods
  - fires callback/interrupt each period

Text diagram:

```
ALSA ring buffer (memory)
|----P0----|----P1----|----P2----|----P3----|
   ^ IRQ         ^ IRQ        ^ IRQ        ^ IRQ
   period elapsed callbacks wake ALSA/app
```

## How interrupts connect to periods (the timing backbone)
At each period boundary:
- DMA completes a chunk
- driver’s DMA callback runs
- ALSA is notified: “period elapsed”
- user space may be woken to write more frames

If callbacks are late or app can’t keep up:
- playback underrun (XRUN)

## Common driver bugs (real projects + interview)
- **Wrong pointer math**: bytes vs frames mismatch → drift, XRUNs, broken `aplay`
- **Clock misconfiguration**: wrong BCLK/LRCLK ratio → noise, speed/pitch errors
- **Incorrect format**: 24-bit packed vs 32-bit container confusion
- **Missing constraints**: userspace selects unsupported params → failure later
- **Not handling STOP/PAUSE properly**: pop noise, stuck DMA, wakeups never resume
- **Cache coherency** (some systems): DMA buffers not coherent → random noise/glitches

## Practical checklist when bringing up a new PCM driver
- Confirm hardware clocks:
  - MCLK/BCLK/LRCLK frequencies match expected for chosen rate/format
- Validate I2S/TDM slot configuration:
  - word length, slot width, channel mapping
- Verify DMA:
  - cyclic DMA works, period callbacks fire at expected cadence
- Use known-good test vectors:
  - 1 kHz sine wave WAV at 48kHz S16_LE stereo
- Test from simplest to complex:
  - `aplay -D hw:X,Y test.wav` (avoid plugins first)

