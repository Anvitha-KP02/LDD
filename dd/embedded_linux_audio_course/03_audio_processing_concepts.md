=== 3. AUDIO PROCESSING CONCEPTS ===

## Frames, samples

Definitions (ALSA-style thinking):
- **Sample**: one numeric value for one channel at one time instant.
- **Frame**: a set of samples across all channels at one time instant.

Example (stereo):
- frame \(n\) = (L[n], R[n])
- If format is `S16_LE`, each sample is 2 bytes
- Each frame is 2 channels × 2 bytes = 4 bytes

Why it matters:
- ALSA APIs often use **frames** as the unit (not bytes).
- Many bugs are “bytes vs frames” mistakes.

## Interleaved vs non-interleaved

### Interleaved (most common)
Samples for channels are stored alternating in one buffer:

```
L0 R0 L1 R1 L2 R2 ...
```

Pros:
- Matches what many DMA engines and I2S/TDM serializers expect
- Simple single-buffer handling

### Non-interleaved (planar)
Each channel has its own buffer:

```
L: L0 L1 L2 ...
R: R0 R1 R2 ...
```

Pros:
- Sometimes easier for per-channel DSP
Cons:
- Often requires reformatting before hardware (extra CPU/mem)

ALSA note:
- ALSA supports both via different access types (`RW_INTERLEAVED`, `RW_NONINTERLEAVED`, `MMAP_*`).

## Buffering

Buffering exists because:
- CPU and hardware run asynchronously
- Decoders produce bursts
- Scheduling jitter (Linux is not a hard real-time OS by default)

Typical buffering layers:

```
App (decode) -> ALSA lib -> kernel PCM -> DMA buffer -> codec -> speaker
```

In PCM playback, ALSA uses a **ring buffer** model:
- The application writes PCM into a circular buffer.
- The hardware reads from it at a constant rate.

Text diagram (ring buffer):

```
|------------------- buffer -------------------|
^hw read ptr                          ^app write ptr
```

Key concept: avoid underrun/overrun
- **Underrun (XRUN)**: playback ran out of samples → glitch/click, stream stops or recovers
- **Overrun**: capture buffer overflowed → lost recorded samples

## Latency
Latency = time from “audio generated” to “sound heard”.

Contributors:
- Codec algorithmic delay (MP3/AAC frame sizes)
- App buffering (queue depth)
- ALSA buffer/period settings
- Kernel scheduling
- DMA period interrupt cadence
- Codec internal pipelines

Rule of thumb:
- Larger buffers → safer from underruns but more latency
- Smaller buffers → lower latency but higher XRUN risk

Simple latency estimate (PCM path, ignoring codec):
- If ALSA buffer holds \(N\) frames at \(F_s\):
  - Latency contribution ≈ \(N / F_s\) seconds

Example:
- buffer_size = 4800 frames at 48kHz → 0.1 s = 100 ms

Embedded interview angle:
- You tune **period size** and **buffer size** for the target:
  - Voice/interactive: aim low latency (e.g., 10–30 ms), requires careful tuning and stable clocks
  - Media playback: higher latency is acceptable; prioritize robustness

