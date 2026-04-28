#!/usr/bin/env python3
"""
Play video files (MP4, MKV, AVI, WEBM, …) on ILI9225 via V4L2 output (/dev/videoX).
All decode, scale, and RGB565 conversion run in userspace (ffmpeg or GStreamer).
"""
from __future__ import annotations

import argparse
import os
import pathlib
import shutil
import subprocess
import sys
from typing import Optional


FORMATS = (".mp4", ".mkv", ".avi", ".webm", ".mov", ".m4v", ".ogv")


def pick_video(path: str) -> Optional[str]:
    if os.path.isfile(path) and path.lower().endswith(FORMATS):
        return os.path.abspath(path)
    if os.path.isdir(path):
        files = sorted(
            f for f in os.listdir(path) if f.lower().endswith(FORMATS)
        )
        if not files:
            return None
        print("Select file:")
        for i, name in enumerate(files, 1):
            print(f"  {i}. {name}")
        while True:
            try:
                c = input("Number (or q): ").strip()
                if c.lower() == "q":
                    return None
                idx = int(c) - 1
                if 0 <= idx < len(files):
                    return os.path.abspath(os.path.join(path, files[idx]))
            except ValueError:
                pass
            print("Invalid choice.")
    return None


def run_ffmpeg(device: str, video: str, realtime: bool) -> int:
    ff = shutil.which("ffmpeg")
    if not ff:
        print("ffmpeg not found", file=sys.stderr)
        return 127
    cmd = [
        ff,
        "-hide_banner",
        "-loglevel",
        "warning",
    ]
    if realtime:
        cmd.append("-re")
    cmd += [
        "-i",
        video,
        "-vf",
        "scale=176:220:flags=fast_bilinear",
        "-pix_fmt",
        "rgb565le",
        "-f",
        "v4l2",
        device,
    ]
    print("Running:", " ".join(cmd))
    return subprocess.call(cmd)


def run_gstreamer(device: str, video: str, realtime: bool) -> int:
    gst = shutil.which("gst-launch-1.0")
    if not gst:
        print("gst-launch-1.0 not found", file=sys.stderr)
        return 127
    uri = pathlib.Path(video).expanduser().resolve().as_uri()
    sync_flag = "true" if realtime else "false"
    cmd = [
        gst,
        "-e",
        "uridecodebin",
        f"uri={uri}",
        "name=d",
        "d.",
        "!",
        "queue",
        "!",
        "videoconvert",
        "!",
        "videoscale",
        "!",
        "video/x-raw,width=176,height=220,format=RGB16",
        "!",
        "v4l2sink",
        f"device={device}",
        f"sync={sync_flag}",
    ]
    print("Running:", " ".join(cmd))
    return subprocess.call(cmd)


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Decode video in userspace and send RGB565 to ILI9225 V4L2 device."
    )
    ap.add_argument(
        "-d",
        "--device",
        default="/dev/video0",
        help="V4L2 output device (default: /dev/video0)",
    )
    ap.add_argument(
        "-i",
        "--input",
        default=".",
        help="Video file or directory to pick from (default: cwd)",
    )
    ap.add_argument(
        "-b",
        "--backend",
        choices=("ffmpeg", "gst"),
        default="ffmpeg",
        help="ffmpeg (default) or gst (GStreamer)",
    )
    ap.add_argument(
        "-r",
        "--realtime",
        action="store_true",
        help="Pace input like live (ffmpeg: -re; GStreamer: sync=true)",
    )
    args = ap.parse_args()

    if not os.access(args.device, os.W_OK):
        print(
            f"Cannot write {args.device} — check path and permissions.",
            file=sys.stderr,
        )
        return 1

    video = pick_video(args.input)
    if not video:
        print("No suitable video file.", file=sys.stderr)
        return 1

    if args.backend == "ffmpeg":
        return run_ffmpeg(args.device, video, args.realtime)
    return run_gstreamer(args.device, video, args.realtime)


if __name__ == "__main__":
    sys.exit(main())
