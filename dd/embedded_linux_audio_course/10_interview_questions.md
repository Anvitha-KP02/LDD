=== 10. INTERVIEW QUESTIONS ===

## Important questions from this topic

### Fundamentals
- What is the difference between **sample** and **frame** in PCM audio?
- Explain **sampling rate** and **bit depth**. How do they affect quality and data rate?
- What is aliasing? Why do we need an anti-alias filter before ADC?
- Compute bandwidth: What is the byte rate for 48kHz stereo 16-bit PCM?

### Formats and pipelines
- What is the difference between **WAV** and **RAW PCM**?
- What is the difference between a **container** and a **codec**?
- Explain the flow: **MP3 → decode → PCM → ALSA → driver → hardware**.
- Where does decoding happen in Linux audio typically (user space vs kernel)?

### ALSA and Linux audio architecture
- What does ALSA provide in kernel vs user space?
- Difference between `hw:0,0`, `plughw:0,0`, and `default`.
- Explain what an **XRUN** is and how you recover from it.
- How do **period size** and **buffer size** influence latency and robustness?

### Driver-level / embedded
- What does an ALSA PCM driver implement?
- Explain the purpose of callbacks:
  - `open`, `hw_params`, `prepare`, `trigger`, `pointer`
- Why do we use DMA for audio?
- How does “period elapsed” relate to interrupts and user-space wakeups?
- What happens if clocks (BCLK/LRCLK) are wrong?

### Raspberry Pi / ASoC
- What is ASoC and why does embedded audio use it?
- What are **CPU DAI**, **Codec DAI**, and **machine driver/DT**?
- Common debug steps when audio works on HDMI but not on I2S codec (or vice versa).

## Common mistakes (and what to say in interviews)

- **Bytes vs frames confusion**
  - Mistake: calling ALSA APIs with byte counts where frames are expected.
  - Correct: track units carefully; convert using `frame_bytes = channels * bytes_per_sample`.

- **Assuming RAW PCM is self-describing**
  - Mistake: “audio.raw should just play.”
  - Correct: you must supply `-f/-c/-r` when playing/recording raw PCM.

- **Using `default` while debugging drivers**
  - Mistake: debugging through conversion/mixing layers.
  - Correct: use `-D hw:X,Y` first to test real hardware constraints.

- **Ignoring mixer routes**
  - Mistake: PCM stream is active but output is muted or muxed away.
  - Correct: verify `amixer/alsamixer`, codec routes, and DAPM widgets.

- **Wrong clocking assumptions**
  - Mistake: codec configured for 48k but stream is 44.1k (or vice versa).
  - Correct: ensure PLL/DAI clocks match the chosen sample rate and word sizes.

- **Too aggressive low-latency settings**
  - Mistake: tiny periods cause XRUNs due to scheduler jitter.
  - Correct: tune period/buffer, consider RT scheduling, reduce load, verify DMA cadence.

## Rapid-fire questions (good for practice)
- Define: PCM, I2S, TDM, DMA, period, buffer, XRUN.
- Why would 44.1kHz audio sound wrong on a 48kHz-only pipeline?
- How do you identify which ALSA card is HDMI vs I2S codec?
- What tools do you use first on a new board bring-up? (aplay/arecord/amixer/dmesg)

