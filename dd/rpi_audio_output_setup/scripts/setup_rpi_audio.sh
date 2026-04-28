#!/usr/bin/env bash
set -euo pipefail

CFG_NEW="/boot/firmware/config.txt"
CFG_OLD="/boot/config.txt"

if [[ -f "${CFG_NEW}" ]]; then
  CFG="${CFG_NEW}"
elif [[ -f "${CFG_OLD}" ]]; then
  CFG="${CFG_OLD}"
else
  echo "Could not find config.txt" >&2
  exit 1
fi

echo "Using config: ${CFG}"
sudo cp "${CFG}" "${CFG}.bak.$(date +%Y%m%d_%H%M%S)"

if ! grep -q '^dtparam=audio=on' "${CFG}"; then
  echo "dtparam=audio=on" | sudo tee -a "${CFG}" >/dev/null
fi

if ! grep -q '^dtoverlay=vc4-kms-v3d' "${CFG}"; then
  echo "dtoverlay=vc4-kms-v3d" | sudo tee -a "${CFG}" >/dev/null
fi

echo "Updated ${CFG}. Reboot required."
echo "Run: sudo reboot"
