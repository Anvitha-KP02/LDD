# Raspberry Pi Real Audio Output Setup (Existing Drivers Only)

This folder gives you a practical, no-custom-kernel-driver setup for Raspberry Pi audio output via:
- 3.5mm jack (analog PWM audio, `snd_bcm2835`)
- HDMI audio (`vc4` HDMI audio path)

It includes:
- step-by-step execution commands
- `config.txt` settings
- ALSA debug commands
- user-space C player for WAV/RAW playback
- optional DTS overlay source (not mandatory for normal jack/HDMI audio)

---

## === AUDIO ARCHITECTURE (RPi) ===

- **3.5mm jack (analog)**:
  - Uses Raspberry Pi onboard analog audio path.
  - Main ALSA kernel module is typically `snd_bcm2835`.
  - Must enable `dtparam=audio=on`.

- **HDMI audio**:
  - Provided through VC4/KMS display stack.
  - Uses `vc4`/DRM path with HDMI PCM devices.
  - With modern Raspberry Pi OS, keep KMS enabled (`vc4-kms-v3d` or fake KMS on older setups).

- **Userspace stack**:
  - ALSA (`aplay`, `amixer`, `alsamixer`, libasound).
  - Optional conversion tools (`ffmpeg`).

---

## === DEVICE TREE / CONFIG.TXT SETTINGS ===

Edit:
```bash
sudo nano /boot/firmware/config.txt
```
On some older images:
```bash
sudo nano /boot/config.txt
```

Recommended lines:

```ini
# Enable onboard analog (3.5mm) audio
dtparam=audio=on

# Enable VC4 KMS graphics/audio stack (Bookworm/Bullseye typical)
dtoverlay=vc4-kms-v3d
```

After save:
```bash
sudo reboot
```

Notes:
- Do not add multiple conflicting VC4 overlays at once.
- For very old images using fake KMS, `dtoverlay=vc4-fkms-v3d` may be used instead.

---

## === DRIVER DETAILS ===

- **Jack/analog path**:
  - Kernel module: `snd_bcm2835`
  - Usually appears as card with names like `bcm2835 Headphones` or similar.

- **HDMI path**:
  - VC4/DRM HDMI audio devices exposed as ALSA playback devices.
  - Card names may include `vc4-hdmi-*` or `bcm2835 HDMI`.

- This setup uses existing upstream/Raspberry Pi kernel drivers only.

---

## === ALSA COMMANDS ===

List cards/devices:
```bash
aplay -l
aplay -L
cat /proc/asound/cards
```

Mixer controls:
```bash
amixer scontrols
amixer -c 0 scontrols
alsamixer
```

Choose output interactively:
- Run `alsamixer`
- Press `F6` to select sound card
- Ensure channels are not muted (`M` toggles mute)

Set volume quickly (example card 0):
```bash
amixer -c 0 set Master 80% unmute
amixer -c 0 set PCM 90% unmute
```

---

## === PLAYBACK COMMANDS ===

### 1) WAV playback
```bash
aplay -D hw:0,0 test.wav
```
If that card is HDMI, switch to another device index from `aplay -l`.

### 2) RAW PCM playback (S16_LE, stereo, 44.1kHz)
```bash
aplay -D hw:0,0 -f S16_LE -c 2 -r 44100 audio.raw
```

### 3) Use provided C player (`src/rpi_audio_player.c`)
Build:
```bash
cd /home/pi/rpi_audio_output_setup
make
```
Run WAV:
```bash
./rpi_audio_player --wav test.wav --device hw:0,0
```
Run RAW:
```bash
./rpi_audio_player --raw audio.raw --rate 44100 --channels 2 --format s16_le --device hw:0,0
```

---

## === MP3 SUPPORT ===

Install tools:
```bash
sudo apt update
sudo apt install -y ffmpeg alsa-utils
```

Direct MP3 play (ffplay):
```bash
ffplay -nodisp -autoexit song.mp3
```

Convert MP3 -> WAV -> play:
```bash
ffmpeg -y -i song.mp3 song.wav
aplay -D hw:0,0 song.wav
```

Convert MP3 -> RAW S16_LE and play:
```bash
ffmpeg -y -i song.mp3 -f s16le -acodec pcm_s16le -ac 2 -ar 44100 song.raw
aplay -D hw:0,0 -f S16_LE -c 2 -r 44100 song.raw
```

---

## === DEBUGGING ===

Check modules:
```bash
lsmod | grep -E "snd|vc4|bcm2835"
```

Check kernel logs:
```bash
dmesg | grep -Ei "alsa|snd|bcm2835|vc4|hdmi|audio"
```

Service sanity:
```bash
systemctl --user status pipewire pipewire-pulse wireplumber 2>/dev/null || true
```

Quick card test:
```bash
speaker-test -D hw:0,0 -c 2 -t sine -f 440
```

---

## === COMMON ISSUES & FIXES ===

1) **No cards listed in `aplay -l`**
- Re-check `config.txt` (`dtparam=audio=on`)
- Reboot after changes
- Check `dmesg` for overlay/driver errors

2) **Sound from wrong output**
- Use explicit device with `-D hw:X,Y`
- Try card index from `aplay -l`
- In desktop: audio output may be routed by PipeWire/PulseAudio; set correct sink

3) **Muted output**
- Open `alsamixer`, unmute channels (`M`)
- Increase volume via `amixer`

4) **HDMI no sound**
- Check monitor/TV actually supports audio
- Try other HDMI port/device index
- Ensure VC4 overlay active

5) **RAW playback sounds noisy**
- Format mismatch. Ensure `-f`, `-c`, `-r` match how RAW was created

---

## Files in this folder

- `src/rpi_audio_player.c` : ALSA user-space WAV/RAW player
- `Makefile` : builds user-space player only
- `dts/rpi_existing_audio_optional_overlay.dts` : optional overlay source
- `scripts/*.sh` : setup, switch, playback, and debug helpers

This approach gives real output on jack/HDMI using Raspberry Pi kernel drivers with no custom kernel audio driver.
