#!/bin/sh
# Example FFmpeg commands: decode + scale + rgb565le → V4L2 output (ILI9225 driver).
# Replace VIDEO and DEV as needed.

VIDEO="${1:-clip.mp4}"
DEV="${2:-/dev/video0}"

set -e

echo "=== MP4 / MKV / AVI / WEBM: same CLI; demuxer is chosen from container ==="

ffmpeg -hide_banner -loglevel warning -re -i "$VIDEO" \
	-vf "scale=176:220:flags=fast_bilinear" \
	-pix_fmt rgb565le \
	-f v4l2 "$DEV"

# Non-realtime (max throughput / benchmarking):
# ffmpeg -hide_banner -i "$VIDEO" -vf "scale=176:220:flags=fast_bilinear" \
#   -pix_fmt rgb565le -f v4l2 "$DEV"

# Software decode thread hint (helps some codecs):
# ffmpeg -threads 2 -i "$VIDEO" ...

# If colors look wrong vs panel, try byte order in kernel DT (swap-bytes) or
# in ffmpeg: -pix_fmt bgr565le (rare; usually rgb565le matches V4L2 RGB565 LE).
