=== 1. BASICS OF AUDIO ===

## What is sound?
Sound is a **pressure wave** traveling through a medium (air, water, solid). Your ear senses **air pressure variations over time**.

Key ideas:
- **Amplitude**: how strong the pressure variation is (perceived as loudness).
- **Frequency**: how fast it oscillates (perceived as pitch). Unit: Hz (cycles/second).
- **Phase**: timing offset of a waveform relative to another.

Text diagram (pressure vs time):

```
Pressure ^
         |      / \      / \      / \
         |     /   \    /   \    /   \
         |____/     \__/     \__/     \____> time
```

Human hearing (roughly):
- ~20 Hz to 20 kHz (varies with age and environment)

In Embedded Linux projects, sound becomes **numbers** (samples) so software can move/process it.

## Analog vs Digital audio

### Analog audio
- Continuous signal in time and amplitude (e.g., microphone voltage).
- Pros: conceptually “continuous”.
- Cons: noise, distortion, component tolerance, hard to store/process perfectly.

### Digital audio
- Signal is represented as **discrete-time samples** (numbers).
- Pros: easy to store, process, compress, transmit; robust to noise after ADC.
- Cons: must pick sampling rate + bit depth; conversions add latency/complexity.

Embedded audio hardware chain often looks like:

```
Mic -> (Analog) -> ADC -> (Digital PCM) -> DSP/CPU -> (Digital PCM) -> DAC -> (Analog) -> Speaker
```

## Sampling, Quantization, Bit depth

### Sampling
Sampling is measuring the analog waveform at regular time intervals.
- Sampling period \(T_s\) seconds between samples
- Sampling rate \(F_s = 1/T_s\) samples/sec (Hz)

If you sample too slowly, you get **aliasing** (high frequencies “fold” into lower ones).

Nyquist idea:
- To represent a frequency \(f_{max}\), you need \(F_s \ge 2 \cdot f_{max}\).
- Real systems also need an **anti-alias filter** before ADC.

### Quantization
Each sample must be rounded to a discrete level. That rounding creates **quantization noise**.

### Bit depth
Bit depth decides how many quantization levels exist:
- 8-bit: 256 levels
- 16-bit: 65,536 levels
- 24-bit: ~16 million levels

Bigger bit depth:
- Better dynamic range (quiet sounds represented better)
- Larger data rate

Rule of thumb dynamic range:
- ~6 dB per bit
- 16-bit ≈ 96 dB, 24-bit ≈ 144 dB (idealized)

In Linux audio, common PCM sample formats:
- `S16_LE` (signed 16-bit little-endian)
- `S24_LE` / `S24_3LE` (packed 24-bit)
- `S32_LE` (signed 32-bit container, often only 24 bits used)

## Sampling rate (8kHz, 44.1kHz, etc.)
Sampling rate sets bandwidth and affects latency/buffering trade-offs.

Common rates and why:
- **8 kHz**: telephony/voice (up to ~4 kHz bandwidth)
- **16 kHz**: better voice quality, speech recognition
- **44.1 kHz**: CD audio (music distribution legacy)
- **48 kHz**: pro/video + common embedded codecs; many SoCs are “native” at 48k
- **96 kHz / 192 kHz**: production, special cases (higher bandwidth, higher CPU/data)

Data-rate example (PCM):
- Stereo, 16-bit, 48 kHz:
  - samples/sec = 48,000 frames/sec
  - each frame = 2 channels × 16 bits = 32 bits = 4 bytes
  - throughput = 48,000 × 4 = **192,000 bytes/sec ≈ 187.5 KiB/s**

Embedded interview angle:
- If your hardware clocking is 48 kHz native and you play 44.1 kHz content, something must do **rate conversion** (SRC) or you’ll get pitch/speed issues.

## Mono vs Stereo
- **Mono**: 1 channel (single stream of samples)
- **Stereo**: 2 channels (Left + Right)
- **Multi-channel**: 4/5.1/7.1 etc (home theater, some automotive)

Important term: **frame**
- One “time instant” across all channels.
- For stereo: one frame = (L sample, R sample).

How stereo PCM is typically stored (interleaved):

```
Frame0: L0 R0
Frame1: L1 R1
Frame2: L2 R2
...
```

Project mindset:
- Always track: **channels, sample format, sample rate**.
- Bugs often come from mismatched assumptions (e.g., treating frames as samples).

