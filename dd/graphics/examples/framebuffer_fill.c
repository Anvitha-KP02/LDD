/*
 * Minimal Linux fbdev example: mmap and fill framebuffer.
 * Requires: /dev/fb0, permission to open it.
 *
 * Teaching focus: fix.line_length (stride), bits_per_pixel, mmap size.
 */

#define _DEFAULT_SOURCE
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

static void put_pixel_xrgb8888(uint8_t *base, int x, int y, int line_len,
			       uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t *row = (uint32_t *)(base + (size_t)y * (size_t)line_len);
	/* XRGB8888: adjust offsets if ioctl reports BGR or different layout */
	row[x] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

int main(void)
{
	int fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		perror("open /dev/fb0");
		return 1;
	}

	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) < 0 ||
	    ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
		perror("ioctl FBIOGET_*");
		close(fd);
		return 1;
	}

	if (vinfo.bits_per_pixel != 32) {
		fprintf(stderr, "This demo expects 32 bpp; got %u\n",
			vinfo.bits_per_pixel);
		close(fd);
		return 1;
	}

	const size_t screensize = (size_t)finfo.line_length * vinfo.yres_virtual;
	uint8_t *fb =
	    mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fb == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return 1;
	}

	const int w = (int)vinfo.xres;
	const int h = (int)vinfo.yres;
	const int stride = (int)finfo.line_length;

	/* Solid fill: orange */
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
			put_pixel_xrgb8888(fb, x, y, stride, 0xff, 0xa5, 0x00);

	/* Small gradient bar: exercise per-pixel write */
	for (int y = 0; y < 40 && y < h; y++) {
		uint8_t r = (uint8_t)((255 * y) / 39);
		for (int x = 0; x < w; x++)
			put_pixel_xrgb8888(fb, x, y, stride, r, 0x40, 0x80);
	}

	munmap(fb, screensize);
	close(fd);
	return 0;
}
