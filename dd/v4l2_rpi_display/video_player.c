/*
 * SPI / framebuffer video player for Raspberry Pi (ILI9341 / ST7789 / fbtft).
 *
 * All demux + decode + pixel conversion run in userspace via FFmpeg libraries.
 * The kernel driver (framebuffer or SPI) only receives complete RGB565 frames.
 *
 * Build: see Makefile (needs libavformat, libavcodec, libswscale, libavutil).
 */

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include <linux/fb.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/rational.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>

#define DEFAULT_FB_PATH "/dev/fb1"
#define DEFAULT_DISP_WIDTH 240
#define DEFAULT_DISP_HEIGHT 320
#define MAX_PATH_LEN 4096
#define MAX_INPUT_LINE 512
#define MAX_LIST_FILES 256

static const char *g_video_ext[] = { ".mp4", ".mkv", ".avi", ".webm", NULL };

/* -------------------------------------------------------------------------- */
/* Small utilities                                                            */
/* -------------------------------------------------------------------------- */

static void trim_newline(char *s)
{
	if (!s)
		return;
	size_t n = strlen(s);
	while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
		s[n - 1] = '\0';
		n--;
	}
}

static int str_cmp_ci(const char *a, const char *b)
{
	if (!a || !b)
		return -1;
	for (; *a && *b; a++, b++) {
		int ca = tolower((unsigned char)*a);
		int cb = tolower((unsigned char)*b);
		if (ca != cb)
			return ca - cb;
	}
	return (unsigned char)*a - (unsigned char)*b;
}

static bool str_endswith_ci(const char *name, const char *suf)
{
	if (!name || !suf)
		return false;
	size_t ln = strlen(name);
	size_t ls = strlen(suf);
	if (ls > ln)
		return false;
	return str_cmp_ci(name + ln - ls, suf) == 0;
}

static bool is_supported_video_ext(const char *ext)
{
	if (!ext)
		return false;
	if (ext[0] != '.')
		return false;
	for (int i = 0; g_video_ext[i]; i++) {
		if (str_cmp_ci(ext, g_video_ext[i]) == 0)
			return true;
	}
	return false;
}

/* User typed a format token: ".mp4", "mp4", ".MKV" */
static bool input_is_format_mode(const char *raw)
{
	if (!raw || !raw[0])
		return false;
	char low[MAX_INPUT_LINE];
	size_t j = 0;
	for (size_t i = 0; raw[i] && j < sizeof(low) - 1; i++)
		low[j++] = (char)tolower((unsigned char)raw[i]);
	low[j] = '\0';
	if (low[0] == '.' && is_supported_video_ext(low))
		return true;
	/* bare extension without dot */
	for (int i = 0; g_video_ext[i]; i++) {
		const char *e = g_video_ext[i] + 1; /* skip dot */
		if (strcmp(low, e) == 0)
			return true;
	}
	return false;
}

static bool build_ext_from_input(const char *raw, char *out_ext, size_t out_sz)
{
	char low[MAX_INPUT_LINE];
	size_t j = 0;
	for (size_t i = 0; raw[i] && j < sizeof(low) - 1; i++)
		low[j++] = (char)tolower((unsigned char)raw[i]);
	low[j] = '\0';
	if (low[0] != '.') {
		if (strlen(low) + 2 > out_sz)
			return false;
		out_ext[0] = '.';
		strncpy(out_ext + 1, low, out_sz - 2);
		out_ext[out_sz - 1] = '\0';
	} else {
		strncpy(out_ext, low, out_sz - 1);
		out_ext[out_sz - 1] = '\0';
	}
	return is_supported_video_ext(out_ext);
}

static double monotonic_seconds(void)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
		return av_gettime_relative() / 1000000.0;
	return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static void sleep_until(double target_t)
{
	for (;;) {
		double now = monotonic_seconds();
		if (now >= target_t)
			return;
		double dt = target_t - now;
		struct timespec rq;

		rq.tv_sec = (time_t)dt;
		rq.tv_nsec = (long)((dt - (double)rq.tv_sec) * 1e9);
		if (rq.tv_nsec < 0)
			rq.tv_nsec = 0;
		nanosleep(&rq, NULL);
	}
}

/* -------------------------------------------------------------------------- */
/* File selection                                                             */
/* -------------------------------------------------------------------------- */

struct path_list {
	char paths[MAX_LIST_FILES][MAX_PATH_LEN];
	size_t count;
};

static void path_list_clear(struct path_list *pl)
{
	pl->count = 0;
}

static int path_list_add(struct path_list *pl, const char *path)
{
	if (pl->count >= MAX_LIST_FILES)
		return -1;
	strncpy(pl->paths[pl->count], path, MAX_PATH_LEN - 1);
	pl->paths[pl->count][MAX_PATH_LEN - 1] = '\0';
	pl->count++;
	return 0;
}

static int cmp_paths(const void *a, const void *b)
{
	return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static void sort_path_list(struct path_list *pl)
{
	const char *ptrs[MAX_LIST_FILES];
	char tmp_paths[MAX_LIST_FILES][MAX_PATH_LEN];

	if (pl->count < 2)
		return;
	for (size_t i = 0; i < pl->count; i++)
		ptrs[i] = pl->paths[i];
	qsort(ptrs, pl->count, sizeof(ptrs[0]), cmp_paths);
	for (size_t i = 0; i < pl->count; i++) {
		memcpy(tmp_paths[i], ptrs[i], MAX_PATH_LEN);
		tmp_paths[i][MAX_PATH_LEN - 1] = '\0';
	}
	for (size_t i = 0; i < pl->count; i++)
		memcpy(pl->paths[i], tmp_paths[i], MAX_PATH_LEN);
}

static int collect_by_extension(const char *dir, const char *ext,
				struct path_list *out)
{
	char pattern[MAX_PATH_LEN];

	path_list_clear(out);
	if (snprintf(pattern, sizeof(pattern), "%s/*%s", dir, ext) >= (int)sizeof(pattern))
		return -ENAMETOOLONG;

	glob_t g;

	memset(&g, 0, sizeof(g));
	{
		int gr = glob(pattern, GLOB_NOSORT, NULL, &g);

		if (gr != 0 && gr != GLOB_NOMATCH) {
			globfree(&g);
			return -EIO;
		}
	}
	for (size_t i = 0; i < g.gl_pathc; i++) {
		if (path_list_add(out, g.gl_pathv[i]) < 0)
			break;
	}
	globfree(&g);
	sort_path_list(out);
	return 0;
}

static int collect_by_basename(const char *dir, const char *base,
			       struct path_list *out)
{
	char cand[MAX_PATH_LEN];

	path_list_clear(out);
	if (strchr(base, '/') || strchr(base, '\\'))
		return -EINVAL;

	for (int i = 0; g_video_ext[i]; i++) {
		if (snprintf(cand, sizeof(cand), "%s/%s%s", dir, base, g_video_ext[i]) >= (int)sizeof(cand))
			return -ENAMETOOLONG;
		if (access(cand, R_OK) == 0) {
			if (path_list_add(out, cand) < 0)
				return -ENOMEM;
		}
	}
	sort_path_list(out);
	return 0;
}

static void print_numbered_list(const struct path_list *pl, const char *header)
{
	printf("\n%s\n", header);
	for (size_t i = 0; i < pl->count; i++) {
		const char *base = strrchr(pl->paths[i], '/');

		base = base ? base + 1 : pl->paths[i];
		printf("  [%zu] %s\n", i + 1, base);
	}
}

static int prompt_choice(size_t n)
{
	char line[MAX_INPUT_LINE];
	long v;

	if (n == 0)
		return -1;
	if (n == 1)
		return 0;

	printf("\nEnter number to play (1-%zu), or 0 to cancel: ", n);
	if (!fgets(line, sizeof(line), stdin))
		return -1;
	trim_newline(line);
	errno = 0;
	v = strtol(line, NULL, 10);
	if (errno != 0 || v < 0)
		return -1;
	if (v == 0)
		return -2;
	if (v < 1 || (size_t)v > n)
		return -1;
	return (int)(v - 1);
}

static int interactive_pick_video(const char *search_dir, char *out_path, size_t out_sz)
{
	char line[MAX_INPUT_LINE];
	struct path_list list;
	char ext[16];

	printf("\nEnter video base name (e.g. video1) OR format (.mp4, .mkv, .avi, .webm):\n");
	printf("> ");
	fflush(stdout);
	if (!fgets(line, sizeof(line), stdin)) {
		fprintf(stderr, "No input.\n");
		return -EINVAL;
	}
	trim_newline(line);
	if (!line[0]) {
		fprintf(stderr, "Empty input.\n");
		return -EINVAL;
	}

	if (input_is_format_mode(line)) {
		if (!build_ext_from_input(line, ext, sizeof(ext))) {
			fprintf(stderr, "Unsupported format. Use: %s\n",
				".mp4 .mkv .avi .webm");
			return -EINVAL;
		}
		if (collect_by_extension(search_dir, ext, &list) != 0) {
			perror("collect_by_extension");
			return -EIO;
		}
		if (list.count == 0) {
			fprintf(stderr, "No files matching *%s in %s\n", ext, search_dir);
			return -ENOENT;
		}
		print_numbered_list(&list, "Matching files:");
		int idx = prompt_choice(list.count);

		if (idx == -2) {
			printf("Cancelled.\n");
			return -ECANCELED;
		}
		if (idx < 0) {
			fprintf(stderr, "Invalid choice.\n");
			return -EINVAL;
		}
		strncpy(out_path, list.paths[idx], out_sz - 1);
		out_path[out_sz - 1] = '\0';
		return 0;
	}

	if (collect_by_basename(search_dir, line, &list) != 0) {
		fprintf(stderr, "Invalid name (no path separators).\n");
		return -EINVAL;
	}
	if (list.count == 0) {
		fprintf(stderr, "No file named '%s' with supported extension in %s\n",
			line, search_dir);
		return -ENOENT;
	}
	if (list.count == 1) {
		strncpy(out_path, list.paths[0], out_sz - 1);
		out_path[out_sz - 1] = '\0';
		return 0;
	}

	print_numbered_list(&list, "Multiple matches:");
	int idx = prompt_choice(list.count);

	if (idx == -2) {
		printf("Cancelled.\n");
		return -ECANCELED;
	}
	if (idx < 0) {
		fprintf(stderr, "Invalid choice.\n");
		return -EINVAL;
	}
	strncpy(out_path, list.paths[idx], out_sz - 1);
	out_path[out_sz - 1] = '\0';
	return 0;
}

/* -------------------------------------------------------------------------- */
/* Framebuffer output                                                         */
/* -------------------------------------------------------------------------- */

struct fb_session {
	int fd;
	uint8_t *map;
	size_t map_len;
	uint32_t line_length;
	uint32_t xres;
	uint32_t yres;
};

static void fb_close(struct fb_session *fb)
{
	if (fb->map && fb->map != MAP_FAILED && fb->map_len)
		munmap(fb->map, fb->map_len);
	fb->map = NULL;
	fb->map_len = 0;
	if (fb->fd >= 0)
		close(fb->fd);
	fb->fd = -1;
}

static int fb_open_mmap(const char *path, struct fb_session *fb)
{
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;

	memset(fb, 0, sizeof(*fb));
	fb->fd = -1;

	fb->fd = open(path, O_RDWR);
	if (fb->fd < 0) {
		fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno));
		return -errno;
	}
	if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &finfo) != 0) {
		fprintf(stderr, "FBIOGET_FSCREENINFO: %s\n", strerror(errno));
		fb_close(fb);
		return -errno;
	}
	if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &vinfo) != 0) {
		fprintf(stderr, "FBIOGET_VSCREENINFO: %s\n", strerror(errno));
		fb_close(fb);
		return -errno;
	}

	fb->line_length = finfo.line_length;
	fb->xres = vinfo.xres;
	fb->yres = vinfo.yres;
	fb->map_len = (size_t)finfo.smem_len;
	fb->map = mmap(NULL, fb->map_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);
	if (fb->map == MAP_FAILED) {
		fprintf(stderr, "Framebuffer mmap failed: %s\n", strerror(errno));
		fb_close(fb);
		return -errno;
	}
	printf("Framebuffer: %s (%ux%u, line_len=%u bytes)\n",
	       path, fb->xres, fb->yres, fb->line_length);
	return 0;
}

/*
 * Copy scaled RGB565LE into framebuffer memory (full-screen blit).
 * Uses two complete CPU buffers (double buffering): decode into one, blit to FB.
 */
static void fb_blit_rgb565(struct fb_session *fb, const uint8_t *src,
			   int src_linesize, unsigned width, unsigned height)
{
	uint8_t *dst_base = fb->map;
	unsigned max_h = fb->yres < height ? fb->yres : height;
	unsigned max_w = fb->xres < width ? fb->xres : width;

	if (!dst_base || !src)
		return;

	for (unsigned y = 0; y < max_h; y++) {
		const uint8_t *srow = src + (size_t)y * (size_t)src_linesize;
		uint8_t *drow = dst_base + (size_t)y * (size_t)fb->line_length;

		memcpy(drow, srow, (size_t)max_w * 2u);
	}
}

/* -------------------------------------------------------------------------- */
/* Decode + scale + play                                                       */
/* -------------------------------------------------------------------------- */

static double infer_fps(AVFormatContext *fmt, AVStream *st)
{
	AVRational fr = av_guess_frame_rate(fmt, st, NULL);

	if (fr.num <= 0 || fr.den <= 0)
		fr = st->avg_frame_rate;
	if (fr.num <= 0 || fr.den <= 0)
		fr = st->r_frame_rate;

	double fps = av_q2d(fr);

	if (fps <= 0.0 || fps > 240.0)
		fps = 30.0;
	return fps;
}

static int play_video(const char *filepath, struct fb_session *fb,
		      unsigned disp_w, unsigned disp_h)
{
	AVFormatContext *fmt = NULL;
	AVCodecContext *dec = NULL;
	const AVCodec *codec = NULL;
	AVStream *vst = NULL;
	AVFrame *frame = NULL;
	AVFrame *rgb_a = NULL;
	AVFrame *rgb_b = NULL;
	AVPacket *pkt = NULL;
	struct SwsContext *sws = NULL;
	int ret;
	int vindex = -1;
	double fps = 30.0;
	int64_t frame_no = 0;
	double next_present;
	double frame_dt;

	ret = avformat_open_input(&fmt, filepath, NULL, NULL);
	if (ret < 0) {
		char buf[AV_ERROR_MAX_STRING_SIZE];

		av_strerror(ret, buf, sizeof(buf));
		fprintf(stderr, "avformat_open_input: %s\n", buf);
		return ret;
	}
	ret = avformat_find_stream_info(fmt, NULL);
	if (ret < 0) {
		fprintf(stderr, "avformat_find_stream_info failed\n");
		goto fail;
	}

	vindex = av_find_best_stream(fmt, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
	if (vindex < 0 || !codec) {
		fprintf(stderr, "No video stream found.\n");
		ret = AVERROR_STREAM_NOT_FOUND;
		goto fail;
	}

	vst = fmt->streams[vindex];
	dec = avcodec_alloc_context3(codec);
	if (!dec) {
		ret = AVERROR(ENOMEM);
		goto fail;
	}
	if (avcodec_parameters_to_context(dec, vst->codecpar) < 0) {
		fprintf(stderr, "avcodec_parameters_to_context failed\n");
		ret = AVERROR(EINVAL);
		goto fail;
	}
	/* Prefer low delay for interactive playback */
	dec->flags |= AV_CODEC_FLAG_LOW_DELAY;

	ret = avcodec_open2(dec, codec, NULL);
	if (ret < 0) {
		fprintf(stderr, "avcodec_open2 failed\n");
		goto fail;
	}

	fps = infer_fps(fmt, vst);
	printf("Video: %ux%u, codec=%s, target %ux%u @ %.3f FPS\n",
	       dec->width, dec->height, codec->name, disp_w, disp_h, fps);

	frame = av_frame_alloc();
	rgb_a = av_frame_alloc();
	rgb_b = av_frame_alloc();
	pkt = av_packet_alloc();
	if (!frame || !rgb_a || !rgb_b || !pkt) {
		ret = AVERROR(ENOMEM);
		goto fail;
	}

	rgb_a->format = AV_PIX_FMT_RGB565LE;
	rgb_a->width = (int)disp_w;
	rgb_a->height = (int)disp_h;
	rgb_b->format = AV_PIX_FMT_RGB565LE;
	rgb_b->width = (int)disp_w;
	rgb_b->height = (int)disp_h;

	ret = av_frame_get_buffer(rgb_a, 32);
	if (ret < 0)
		goto fail;
	ret = av_frame_get_buffer(rgb_b, 32);
	if (ret < 0)
		goto fail;

	sws = sws_getContext(dec->width, dec->height, dec->pix_fmt,
			    (int)disp_w, (int)disp_h, AV_PIX_FMT_RGB565LE,
			    SWS_BILINEAR, NULL, NULL, NULL);
	if (!sws) {
		fprintf(stderr, "sws_getContext failed (pixel format %s unsupported?)\n",
			av_get_pix_fmt_name(dec->pix_fmt));
		ret = AVERROR(EINVAL);
		goto fail;
	}

	frame_dt = 1.0 / fps;
	next_present = monotonic_seconds();
	int use_a = 1;

	for (;;) {
		ret = av_read_frame(fmt, pkt);
		if (ret == AVERROR_EOF)
			break;
		if (ret < 0) {
			fprintf(stderr, "av_read_frame error\n");
			av_packet_unref(pkt);
			goto fail;
		}
		if (pkt->stream_index != vindex) {
			av_packet_unref(pkt);
			continue;
		}

		ret = avcodec_send_packet(dec, pkt);
		av_packet_unref(pkt);
		if (ret < 0) {
			if (ret == AVERROR_INVALIDDATA || ret == AVERROR(EAGAIN))
				continue;
			fprintf(stderr, "avcodec_send_packet failed (%d)\n", ret);
			goto fail;
		}

		for (;;) {
			ret = avcodec_receive_frame(dec, frame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				break;
			if (ret < 0) {
				fprintf(stderr, "avcodec_receive_frame failed\n");
				goto fail;
			}

			AVFrame *out_rgb = use_a ? rgb_a : rgb_b;

			ret = av_frame_make_writable(out_rgb);
			if (ret < 0)
				goto fail;

			sws_scale(sws, (const uint8_t *const *)frame->data,
				  frame->linesize, 0, frame->height,
				  out_rgb->data, out_rgb->linesize);

			sleep_until(next_present);
			next_present += frame_dt;
			/* If decode fell far behind, avoid endless catch-up burst */
			{
				double now = monotonic_seconds();

				if (next_present + 0.5 < now)
					next_present = now + frame_dt;
			}

			fb_blit_rgb565(fb, out_rgb->data[0], out_rgb->linesize[0],
				       disp_w, disp_h);
			use_a ^= 1;
			frame_no++;
		}
	}

	/* Flush decoder */
	ret = avcodec_send_packet(dec, NULL);
	if (ret < 0 && ret != AVERROR_EOF)
		/* ignore */;

	for (;;) {
		ret = avcodec_receive_frame(dec, frame);
		if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
			break;
		if (ret < 0)
			goto fail;

		AVFrame *out_rgb = use_a ? rgb_a : rgb_b;

		ret = av_frame_make_writable(out_rgb);
		if (ret < 0)
			goto fail;
		sws_scale(sws, (const uint8_t *const *)frame->data,
			  frame->linesize, 0, frame->height,
			  out_rgb->data, out_rgb->linesize);

		sleep_until(next_present);
		next_present += frame_dt;
		{
			double now = monotonic_seconds();

			if (next_present + 0.5 < now)
				next_present = now + frame_dt;
		}

		fb_blit_rgb565(fb, out_rgb->data[0], out_rgb->linesize[0],
			       disp_w, disp_h);
		use_a ^= 1;
		frame_no++;
	}

	printf("Playback finished (%ld frames).\n", (long)frame_no);
	ret = 0;

fail:
	sws_freeContext(sws);
	av_frame_free(&frame);
	av_frame_free(&rgb_a);
	av_frame_free(&rgb_b);
	av_packet_free(&pkt);
	if (dec)
		avcodec_free_context(&dec);
	if (fmt)
		avformat_close_input(&fmt);
	return ret;
}

/* -------------------------------------------------------------------------- */
/* main                                                                       */
/* -------------------------------------------------------------------------- */

static void usage(const char *argv0)
{
	fprintf(stderr,
		"Usage: %s [-d search_dir] [-f fb_device] [-W width] [-H height] [video_file]\n"
		"  Default framebuffer: %s\n"
		"  Default size: %ux%u (RGB565, must match mode)\n",
		argv0, DEFAULT_FB_PATH, DEFAULT_DISP_WIDTH, DEFAULT_DISP_HEIGHT);
}

int main(int argc, char **argv)
{
	const char *search_dir = ".";
	const char *fb_path = DEFAULT_FB_PATH;
	char video_path[MAX_PATH_LEN];
	unsigned disp_w = DEFAULT_DISP_WIDTH;
	unsigned disp_h = DEFAULT_DISP_HEIGHT;
	struct fb_session fb;
	int opt;

	memset(&fb, 0, sizeof(fb));
	fb.fd = -1;

	while ((opt = getopt(argc, argv, "d:f:W:H:h")) != -1) {
		switch (opt) {
		case 'd':
			search_dir = optarg;
			break;
		case 'f':
			fb_path = optarg;
			break;
		case 'W':
			disp_w = (unsigned)strtoul(optarg, NULL, 10);
			break;
		case 'H':
			disp_h = (unsigned)strtoul(optarg, NULL, 10);
			break;
		case 'h':
			usage(argv[0]);
			return 0;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (disp_w == 0 || disp_h == 0) {
		fprintf(stderr, "Invalid display size.\n");
		return 1;
	}

	if (optind < argc) {
		const char *dot;

		if (strlen(argv[optind]) >= sizeof(video_path)) {
			fprintf(stderr, "Path too long.\n");
			return 1;
		}
		strncpy(video_path, argv[optind], sizeof(video_path) - 1);
		video_path[sizeof(video_path) - 1] = '\0';
		dot = strrchr(video_path, '.');
		if (!dot || !is_supported_video_ext(dot)) {
			fprintf(stderr,
				"Unsupported extension (use .mp4, .mkv, .avi, .webm).\n");
			return 1;
		}
		if (access(video_path, R_OK) != 0) {
			fprintf(stderr, "Cannot read file: %s\n", video_path);
			return 1;
		}
	} else {
		int pr = interactive_pick_video(search_dir, video_path, sizeof(video_path));

		if (pr == -ECANCELED)
			return 0;
		if (pr != 0)
			return 1;
	}

	if (fb_open_mmap(fb_path, &fb) != 0)
		return 1;

	if (disp_w > fb.xres || disp_h > fb.yres)
		fprintf(stderr,
			"Warning: requested %ux%u > fb %ux%u; blit will clip.\n",
			disp_w, disp_h, fb.xres, fb.yres);

	printf("Playing: %s\n", video_path);
	int pret = play_video(video_path, &fb, disp_w, disp_h);

	fb_close(&fb);
	return pret != 0 ? 1 : 0;
}
