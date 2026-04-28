/* SPDX-License-Identifier: MIT */
/*
 * Minimal launcher: ffmpeg decodes to RGB565 and writes to V4L2 output.
 * Build: cc -O2 -o play_to_lcd play_to_lcd.c
 * Usage: ./play_to_lcd /dev/video0 /path/to/video.mp4
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int file_readable(const char *p)
{
	return access(p, R_OK) == 0;
}

int main(int argc, char **argv)
{
	const char *dev = "/dev/video0";
	const char *path;

	if (argc >= 3) {
		dev = argv[1];
		path = argv[2];
	} else if (argc == 2) {
		path = argv[1];
	} else {
		fprintf(stderr, "usage: %s [ /dev/videoN ] /path/to/video\n", argv[0]);
		return 1;
	}

	if (!file_readable(path)) {
		fprintf(stderr, "cannot read %s: %s\n", path, strerror(errno));
		return 1;
	}

	execlp("ffmpeg", "ffmpeg",
	       "-hide_banner", "-loglevel", "warning",
	       "-re", "-i", path,
	       "-vf", "scale=176:220:flags=fast_bilinear",
	       "-pix_fmt", "rgb565le",
	       "-f", "v4l2", dev,
	       (char *)NULL);

	fprintf(stderr, "execlp(ffmpeg): %s\n", strerror(errno));
	return 127;
}
