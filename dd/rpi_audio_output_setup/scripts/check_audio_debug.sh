#!/usr/bin/env bash
set -euo pipefail

echo "=== ALSA cards ==="
cat /proc/asound/cards || true
echo

echo "=== Playback devices (aplay -l) ==="
aplay -l || true
echo

echo "=== ALSA PCMs (aplay -L) ==="
aplay -L | sed -n '1,80p' || true
echo

echo "=== Loaded modules ==="
lsmod | grep -E "snd|vc4|bcm2835|drm" || true
echo

echo "=== dmesg audio snippets ==="
dmesg | grep -Ei "alsa|snd|bcm2835|vc4|hdmi|audio" | tail -n 120 || true
