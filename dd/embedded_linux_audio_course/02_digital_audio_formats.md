=== 2. DIGITAL AUDIO FORMATS ===

This section separates two things people often mix up:
- **Audio coding (PCM vs compressed)**: what the samples “mean”
- **Container/file format (WAV, MP4, etc.)**: how data + metadata are packaged

## RAW PCM
**RAW PCM** means: “just the samples,” with **no header** and usually no metadata.

To interpret RAW PCM you MUST know:
- Sample rate (e.g., 48000)
- Channels (e.g., 2)
- Sample format (e.g., signed 16-bit little-endian `S16_LE`)
- Interleaving layout (usually interleaved)

Example:
- A file `audio.raw` could be 48kHz stereo S16_LE… or something else. The file itself doesn’t say.

Embedded use:
- DMA and drivers fundamentally move **PCM** (or TDM PCM) buffers.

## WAV
**WAV** is a container (RIFF) that commonly stores **PCM** (uncompressed) and includes headers describing:
- Sample rate, channels, bit depth
- Sometimes extra chunks (metadata)

Common: PCM in WAV (simple, friendly for debugging).

Why embedded engineers like WAV:
- You can verify capture/playback quickly with known-good tooling.
- It makes data self-describing compared to RAW.

## MP3, AAC, FLAC

### MP3 (lossy)
- Uses perceptual coding: throws away information considered less audible.
- Good compression ratio; widely supported.
- Decoding costs CPU and adds buffering/latency considerations.

### AAC (lossy)
- Newer than MP3, generally better quality at same bitrate.
- Common in streaming, Bluetooth stacks, mobile pipelines.

### FLAC (lossless)
- Compresses without losing information (bit-perfect reconstruction of PCM).
- Bigger than MP3/AAC but smaller than raw PCM/WAV.

Embedded reality:
- Most SoCs play compressed formats by decoding in **user space** (CPU) or via **hardware codecs**.
- Kernel typically does **not** decode MP3/AAC/FLAC; it moves PCM to/from hardware.

## Differences between lossy and lossless

### Lossy (MP3/AAC/Opus)
- Pros:
  - Much smaller files/streams
  - Lower bandwidth
- Cons:
  - Not bit-perfect; artifacts possible
  - Decode complexity
  - Often adds algorithmic delay (codec frame sizes)

### Lossless (FLAC/ALAC) and Uncompressed (PCM/WAV)
- Pros:
  - Exact recovery of original PCM (lossless)
  - Easier for testing and verifying signal integrity
- Cons:
  - Higher bandwidth/storage vs lossy

Quick comparison table (conceptual):

```
PCM/WAV:  huge size, simplest, best for debugging
FLAC:     medium size, lossless, decode needed
MP3/AAC:  small size, lossy, decode needed
```

Interview-ready distinction:
- **Container** vs **codec**:
  - WAV is a container; it can carry PCM (most common), and sometimes compressed data.
  - MP3 is typically both the codec and a common file bitstream format.
  - MP4/M4A commonly contain AAC (container + codec pairing).

