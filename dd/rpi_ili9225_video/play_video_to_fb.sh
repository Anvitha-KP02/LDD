#!/usr/bin/env bash
# Decode almost any video container/codec (ffmpeg + libav) and show on ILI9225 fbdev.
# The ILI9225 DRM driver registers a linear RGB565 framebuffer (176x220).

set -euo pipefail

VID="${1:-}"
FB="${FBDEV:-}"

if [[ -z "$VID" || ! -f "$VID" ]]; then
	echo "Usage: FBDEV=/dev/fbN $0 <video.mkv|mp4|mov|...>" >&2
	exit 1
fi

if [[ -z "$FB" ]]; then
	for n in /dev/fb*; do
		[[ -e "$n" ]] || continue
		if [[ $(cat "/sys/class/graphics/${n#/dev/}/name" 2>/dev/null) == *ili9225* ]]; then
			FB="$n"
			break
		fi
	done
fi

if [[ -z "$FB" ]]; then
	FB="/dev/fb0"
	echo "Warning: ili9225 fb not auto-detected; using $FB. Set FBDEV= if wrong." >&2
fi

W=176
H=220

exec ffmpeg -hide_banner -loglevel warning -i "$VID" \
	-vf "scale=${W}:${H}:force_original_aspect_ratio=decrease,pad=${W}:${H}:(ow-iw)/2:(oh-ih)/2" \
	-pix_fmt rgb565le \
	-f fbdev -bpp 16 "$FB"
