#!/bin/sh
# GStreamer → v4l2sink at 176x220 RGB16 (RGB565-friendly).
# apt install gstreamer1.0-tools gstreamer1.0-libav gstreamer1.0-plugins-good
#
# --- Example 1: uridecodebin (MP4/MKV/AVI/WEBM and more) ---
# gst-launch-1.0 -e uridecodebin uri=file:///path/to/video.mp4 name=d \
#   d. ! queue ! videoconvert ! videoscale ! video/x-raw,width=176,height=220,format=RGB16 ! \
#   v4l2sink device=/dev/video0 sync=false
#
# --- Example 2: playbin (simple; set video-sink pipeline) ---
# gst-launch-1.0 -e playbin uri=file:///path/to/video.mkv \
#   video-sink="videoconvert ! videoscale ! video/x-raw,width=176,height=220,format=RGB16 ! v4l2sink device=/dev/video0 sync=true"
#
# --- Example 3: leaky queue (favor freshness over backlog if SPI is slow) ---
#   d. ! queue max-size-buffers=2 leaky=downstream ! videoconvert ! ...

set -e
URI="${1:-file://$(pwd)/clip.mp4}"
DEV="${2:-/dev/video0}"

exec gst-launch-1.0 -e \
	uridecodebin uri="$URI" name=d \
	d. ! queue ! videoconvert ! videoscale ! \
	video/x-raw,width=176,height=220,format=RGB16 ! \
	v4l2sink device="$DEV" sync=false
