#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 ]]; then
  echo "Usage:"
  echo "  $0 wav <file.wav> [device hw:0,0]"
  echo "  $0 raw <file.raw> [device hw:0,0]"
  echo "  $0 mp3 <file.mp3> [device hw:0,0]"
  exit 1
fi

MODE="$1"
FILE="$2"
DEV="${3:-hw:0,0}"

case "${MODE}" in
  wav)
    aplay -D "${DEV}" "${FILE}"
    ;;
  raw)
    aplay -D "${DEV}" -f S16_LE -c 2 -r 44100 "${FILE}"
    ;;
  mp3)
    TMP_WAV="/tmp/rpi_audio_play.wav"
    ffmpeg -y -i "${FILE}" "${TMP_WAV}"
    aplay -D "${DEV}" "${TMP_WAV}"
    ;;
  *)
    echo "Unknown mode: ${MODE}" >&2
    exit 1
    ;;
esac
