=== 7. RASPBERRY PI AUDIO ===

Raspberry Pi is a great embedded-audio learning platform because it has multiple audio “routes” and uses standard Linux audio subsystems.

## How audio works in Raspberry Pi (high-level)
Your audio stream is still:
PCM in user space → ALSA → kernel driver → hardware output

The main difference is which hardware block is used:
- PWM/analog (older models)
- HDMI audio
- I2S to an external codec (best for embedded-quality audio)
- USB audio dongles (common in prototyping)

Text diagram:

```
ALSA PCM -> (choose output path)
   |-> HDMI audio block -> HDMI sink (TV/monitor)
   |-> I2S controller -> external codec (I2C control + I2S data) -> speakers
   |-> (older) PWM + filter -> 3.5mm jack
   |-> USB audio class device -> external DAC
```

## Headphone jack vs HDMI

### 3.5mm “headphone” jack (model dependent)
Historically on some Pi models, the 3.5mm output was produced using **PWM** (not a true high-quality DAC), then filtered.
- Pros: built-in, simple
- Cons: quality limitations, noise, depends heavily on board design

Newer Pi models have changed audio capabilities; always check your specific board.

### HDMI audio
HDMI carries digital audio. The Pi outputs PCM over HDMI to the monitor/TV which does DAC.
- Pros: digital, often good quality
- Cons: depends on sink capabilities; routing/EDID can be confusing

Embedded tip:
- When debugging, identify which ALSA card/device maps to HDMI vs analog/I2S.

## snd_bcm2835 driver (what it is)
On Raspberry Pi, `snd_bcm2835` is a commonly seen ALSA driver module historically associated with onboard audio paths.

What to know for interviews/projects:
- It exposes an ALSA card/device that user space can open with `aplay`.
- It’s not “decoding MP3” — it’s a PCM endpoint.
- It interacts with Pi-specific audio hardware blocks (implementation details depend on Pi generation).

To see drivers/cards:
- `aplay -l` (list PCM devices)
- `cat /proc/asound/cards` (cards overview)

## ASoC overview (why it matters on Pi)
For “proper embedded audio” (external codec on I2S), you typically use ASoC:
- **CPU DAI**: I2S controller on the SoC
- **Codec DAI**: external audio codec chip
- **DAI link (machine driver)**: describes how CPU and codec connect and what clocks/routes exist

ASoC building blocks:
- **Codec driver**: controls codec registers (I2C/SPI), mixers, power states
- **CPU DAI driver**: controls SoC audio interface (I2S/TDM)
- **Machine driver / Device Tree**: ties them together (routing, widgets, clocks)

ASoC text diagram:

```
        [Machine / DT]
             |
   -------------------------
   |                       |
[CPU DAI] <---I2S/TDM---> [Codec DAI]
   |                       |
SoC I2S regs            Codec regs (I2C)
```

Embedded project best practice on Pi:
- Prefer I2S + external codec (I2S HAT) for deterministic clocks and better analog performance.
- Use Device Tree overlays to enable and configure the sound card.

