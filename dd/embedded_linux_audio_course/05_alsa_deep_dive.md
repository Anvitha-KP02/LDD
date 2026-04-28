=== 5. ALSA DEEP DIVE ===

This section is focused on **ALSA PCM** because that’s what you use for playback/capture and what drivers ultimately implement.

## snd_pcm_* APIs (what you should know)

Core objects:
- `snd_pcm_t *handle`: your PCM device instance
- `snd_pcm_hw_params_t`: hardware parameters
- `snd_pcm_sw_params_t`: software parameters

Common functions (playback/capture):
- `snd_pcm_open()`: open PCM device
- `snd_pcm_hw_params_any()`: init params with full capability set
- `snd_pcm_hw_params_set_access()`: interleaved/non-interleaved/mmap
- `snd_pcm_hw_params_set_format()`: `S16_LE`, `S32_LE`, etc.
- `snd_pcm_hw_params_set_rate_near()`: sample rate
- `snd_pcm_hw_params_set_channels()`: channel count
- `snd_pcm_hw_params_set_period_size_near()`: frames per period
- `snd_pcm_hw_params_set_buffer_size_near()`: total ring buffer frames
- `snd_pcm_hw_params()`: apply hardware params
- `snd_pcm_prepare()`: prepare stream for I/O
- `snd_pcm_writei()` / `snd_pcm_readi()`: interleaved I/O (frames)
- `snd_pcm_drain()` / `snd_pcm_drop()`: stop behavior
- `snd_pcm_recover()`: handle XRUN/suspend errors
- `snd_pcm_delay()`, `snd_pcm_avail_update()`: timing info

Interview note:
- `writei/readi` use **frames**, not bytes.

## PCM playback flow (typical)

Text diagram of the steady-state loop:

```
open -> hw_params -> (optional sw_params) -> prepare
  |
  v
while (playing):
    write frames
    handle -EPIPE (XRUN) via snd_pcm_recover()
drain/close
```

Minimal pseudo-code (conceptual):

```c
snd_pcm_open(&h, "default", SND_PCM_STREAM_PLAYBACK, 0);
// set hw params: format/rate/channels/period/buffer
snd_pcm_prepare(h);

while (...) {
  frames_written = snd_pcm_writei(h, buf, frames);
  if (frames_written < 0)
    snd_pcm_recover(h, frames_written, 1);
}
snd_pcm_drain(h);
snd_pcm_close(h);
```

## Buffer handling (ring buffer mental model)

Two important sizes (in frames):
- **buffer_size**: total frames in kernel ring buffer
- **period_size**: chunk size that drives wakeups/interrupt cadence

Constraints:
- Typically buffer_size is a multiple of period_size

Why period exists:
- Hardware/DMA moves audio in chunks.
- ALSA uses “period elapsed” events to wake up user space and advance pointers.

Text diagram:

```
|---- period ----|---- period ----|---- period ----|---- period ----|
|------------------------------ buffer ------------------------------|
```

## Period size, buffer size (how to choose)

Trade-offs:
- Smaller period_size:
  - Lower wakeup latency and potentially lower end-to-end latency
  - More interrupts/wakeups → more CPU overhead, more sensitive to jitter
- Larger period_size:
  - More robust
  - Higher latency and coarser timing

Embedded examples (rules of thumb, not absolute):
- Media playback: period 1024–4096 frames @48kHz (≈21–85 ms per period) is common in simple setups
- Low-latency voice: period 120–480 frames @48kHz (≈2.5–10 ms) but demands careful tuning

Simple conversions:
- period_time_ms ≈ (period_size / sample_rate) × 1000
- buffer_time_ms ≈ (buffer_size / sample_rate) × 1000

## Interrupts / DMA concept (how ALSA stays in sync)

Typical embedded hardware path:
- DMA engine reads from memory and feeds an audio peripheral (I2S/TDM).
- The audio peripheral clock is driven by PLLs and must be stable.
- At the end of each period, an interrupt (or DMA callback) signals progress.

Conceptual flow:

```
User writes PCM -> kernel ring buffer -> DMA reads -> I2S shifts bits -> codec
                                   ^ IRQ/period elapsed updates pointer ^
```

Failure modes you’ll debug in real projects:
- Wrong clocking (MCLK/BCLK/LRCLK mismatch) → wrong speed/pitch or noise
- Period too small + scheduler jitter → XRUNs
- Incorrect sample format or channel mapping → distortion/swapped channels

