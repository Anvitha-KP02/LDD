=== 9. PRACTICAL COMMANDS ===

These commands are your “bring-up and debug toolbox” on Embedded Linux.

## aplay (playback)

- List playback devices:
  - `aplay -l`
- List PCM devices including “plugins”:
  - `aplay -L`
- Play a WAV to a specific hardware device:
  - `aplay -D hw:0,0 test.wav`
- Play RAW PCM (you must specify params):
  - `aplay -D hw:0,0 -f S16_LE -c 2 -r 48000 audio.raw`

Common flags:
- `-D`: device
- `-f`: format (`S16_LE`, `S32_LE`, ...)
- `-c`: channels
- `-r`: rate

Debug tip:
- Start with `hw:*` to avoid automatic conversion.

## arecord (capture)

- List capture devices:
  - `arecord -l`
- Record WAV (self-describing) for 5 seconds:
  - `arecord -D hw:0,0 -f S16_LE -c 2 -r 48000 -d 5 out.wav`
- Record RAW PCM:
  - `arecord -D hw:0,0 -f S16_LE -c 1 -r 16000 -d 5 out.raw`

Check what you recorded:
- `aplay out.wav`
- If RAW, you must specify the same params you recorded with:
  - `aplay -f S16_LE -c 1 -r 16000 out.raw`

## amixer (controls, volumes, routes)

ALSA exposes mixer/control elements (volume, mute, muxes, etc.).

- Show sound cards:
  - `amixer -c 0 info`
- List simple mixer controls:
  - `amixer -c 0 scontrols`
- Show a control (example name differs by platform):
  - `amixer -c 0 sget 'Master'`
- Set volume:
  - `amixer -c 0 sset 'Master' 80%`
- Toggle mute:
  - `amixer -c 0 sset 'Master' unmute`
  - `amixer -c 0 sset 'Master' mute`

Embedded tip:
- If you “hear nothing” but PCM streaming looks fine, often it’s a muted control or wrong route/mux.

## alsamixer (interactive UI)

- Start interactive mixer for card 0:
  - `alsamixer -c 0`

Things to do in `alsamixer`:
- Ensure outputs are not muted (look for “MM” vs “OO” depending on UI)
- Verify correct playback/capture source routes if your codec exposes muxes

## Essential inspection commands (high signal)

- See ALSA cards:
  - `cat /proc/asound/cards`
- See PCM devices:
  - `cat /proc/asound/pcm`
- Kernel log for driver issues:
  - `dmesg | tail -n 200`

## Debugging workflow (embedded bring-up)

1) Verify device appears:
   - `aplay -l`, `cat /proc/asound/cards`
2) Test simplest playback:
   - `aplay -D hw:0,0 test.wav`
3) If silent:
   - check `amixer/alsamixer` routes + mute
4) If noisy/distorted:
   - verify rate/format/channels match hardware
   - suspect clocking and slot configuration
5) If XRUNs:
   - increase period/buffer, reduce CPU load, check scheduler/RT priorities

