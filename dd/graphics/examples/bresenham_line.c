/*
 * Bresenham line rasterization into a software RGBA8888 buffer.
 * Pair with framebuffer blit: copy rows respecting fb stride, or draw in
 * a temp buffer then memcpy per row to mapped /dev/fb0.
 *
 * No external deps; compile: gcc -O2 -Wall -o bresenham_line bresenham_line.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct buffer {
	int w, h;
	uint32_t *px; /* RGBA host-endian; for XRGB fb you'd mask alpha */
};

static void put_pixel(struct buffer *b, int x, int y, uint32_t rgba)
{
	if (x < 0 || y < 0 || x >= b->w || y >= b->h)
		return;
	b->px[y * b->w + x] = rgba;
}

/* Integer Bresenham from (x0,y0) to (x1,y1), inclusive */
void draw_line(struct buffer *b, int x0, int y0, int x1, int y1, uint32_t color)
{
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx - dy;

	for (;;) {
		put_pixel(b, x0, y0, color);
		if (x0 == x1 && y0 == y1)
			break;
		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}
}

/* Dump trivial PPM P6 (binary RGB) for visual sanity check */
static void write_ppm(const char *path, const struct buffer *b)
{
	FILE *f = fopen(path, "wb");
	if (!f)
		return;
	fprintf(f, "P6 %d %d 255\n", b->w, b->h);
	for (int y = 0; y < b->h; y++) {
		for (int x = 0; x < b->w; x++) {
			uint32_t p = b->px[y * b->w + x];
			unsigned char rgb[3] = {
				(unsigned char)((p >> 16) & 0xff),
				(unsigned char)((p >> 8) & 0xff),
				(unsigned char)(p & 0xff),
			};
			fwrite(rgb, 1, 3, f);
		}
	}
	fclose(f);
}

int main(void)
{
	struct buffer b = { .w = 320, .h = 240 };
	b.px = calloc((size_t)b.w * (size_t)b.h, sizeof *b.px);
	if (!b.px)
		return 1;

	uint32_t bg = 0x102030ff;
	for (int i = 0; i < b.w * b.h; i++)
		b.px[i] = bg;

	draw_line(&b, 10, 10, 300, 200, 0x00ff66ff);
	draw_line(&b, 300, 30, 20, 220, 0xff3366ff);

	write_ppm("/tmp/bresenham_out.ppm", &b);
	puts("Wrote /tmp/bresenham_out.ppm — view with: xdg-open /tmp/bresenham_out.ppm");

	free(b.px);
	return 0;
}
