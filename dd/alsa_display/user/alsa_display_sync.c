#define _POSIX_C_SOURCE 200809L
#include <alsa/asoundlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "image_array_hanuman.h"

#define WIDTH 176
#define HEIGHT 220
#define FRAME_SIZE_BYTES (WIDTH * HEIGHT * 2)
#define DEFAULT_RATE 44100
#define DEFAULT_CHANNELS 2
#define DEFAULT_PERIOD_FRAMES 1024

struct app_ctx {
	const char *audio_path;
	const char *pcm_device;
	const char *lcd_device;
	atomic_int audio_done;
	double audio_seconds;
};

static int configure_pcm(snd_pcm_t *handle, snd_pcm_uframes_t *period_frames)
{
	snd_pcm_hw_params_t *params;
	unsigned int rate = DEFAULT_RATE;
	int channels = DEFAULT_CHANNELS;
	int rc;

	snd_pcm_hw_params_alloca(&params);

	rc = snd_pcm_hw_params_any(handle, params);
	if (rc < 0)
		return rc;

	rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0)
		return rc;

	rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	if (rc < 0)
		return rc;

	rc = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (rc < 0)
		return rc;

	rc = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
	if (rc < 0)
		return rc;

	rc = snd_pcm_hw_params_set_period_size_near(handle, params, period_frames, 0);
	if (rc < 0)
		return rc;

	{
		snd_pcm_uframes_t buffer_frames = (*period_frames) * 4;
		rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer_frames);
		if (rc < 0)
			return rc;
	}

	return snd_pcm_hw_params(handle, params);
}

static double pcm_duration_seconds_from_file(const char *path)
{
	struct stat st;
	double bytes_per_frame = (double)(DEFAULT_CHANNELS * 2);

	if (stat(path, &st) < 0)
		return 1.0;

	if (st.st_size <= 0)
		return 1.0;

	return ((double)st.st_size / bytes_per_frame) / (double)DEFAULT_RATE;
}

static void *audio_thread(void *arg)
{
	struct app_ctx *ctx = (struct app_ctx *)arg;
	FILE *fp = NULL;
	snd_pcm_t *handle = NULL;
	snd_pcm_uframes_t period_frames = DEFAULT_PERIOD_FRAMES;
	int frame_bytes = DEFAULT_CHANNELS * 2;
	int chunk_bytes;
	uint8_t *buffer = NULL;
	int rc;

	fp = fopen(ctx->audio_path, "rb");
	if (!fp) {
		perror("fopen audio");
		goto out;
	}

	rc = snd_pcm_open(&handle, ctx->pcm_device, SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0) {
		fprintf(stderr, "snd_pcm_open(%s) failed: %s\n", ctx->pcm_device, snd_strerror(rc));
		goto out;
	}

	rc = configure_pcm(handle, &period_frames);
	if (rc < 0) {
		fprintf(stderr, "PCM hw_params failed: %s\n", snd_strerror(rc));
		goto out;
	}

	chunk_bytes = (int)period_frames * frame_bytes;
	buffer = malloc((size_t)chunk_bytes);
	if (!buffer) {
		perror("malloc audio buffer");
		goto out;
	}

	while (1) {
		size_t bytes_read = fread(buffer, 1, (size_t)chunk_bytes, fp);
		snd_pcm_sframes_t frames_to_write;
		snd_pcm_sframes_t written;
		uint8_t *p;

		if (bytes_read == 0)
			break;

		if (bytes_read % frame_bytes) {
			size_t aligned = bytes_read - (bytes_read % frame_bytes);
			bytes_read = aligned;
		}
		if (!bytes_read)
			break;

		frames_to_write = (snd_pcm_sframes_t)(bytes_read / frame_bytes);
		p = buffer;

		while (frames_to_write > 0) {
			written = snd_pcm_writei(handle, p, (snd_pcm_uframes_t)frames_to_write);
			if (written == -EPIPE) {
				snd_pcm_prepare(handle);
				continue;
			}
			if (written < 0) {
				written = snd_pcm_recover(handle, (int)written, 0);
				if (written < 0) {
					fprintf(stderr, "snd_pcm_writei failed: %s\n", snd_strerror((int)written));
					goto out;
				}
				continue;
			}

			frames_to_write -= written;
			p += written * frame_bytes;
		}
	}

	snd_pcm_drain(handle);

out:
	if (buffer)
		free(buffer);
	if (handle)
		snd_pcm_close(handle);
	if (fp)
		fclose(fp);

	atomic_store(&ctx->audio_done, 1);
	return NULL;
}

static void *image_thread(void *arg)
{
	struct app_ctx *ctx = (struct app_ctx *)arg;
	int fd;
	const uint16_t *frames[] = { image1, image2, image3, image4, image5, image6, image7 };
	int frame_count = (int)(sizeof(frames) / sizeof(frames[0]));
	useconds_t frame_delay_us;

	if (ctx->audio_seconds < 0.2)
		ctx->audio_seconds = 0.2;
	frame_delay_us = (useconds_t)((ctx->audio_seconds / frame_count) * 1000000.0);
	if (frame_delay_us < 10000)
		frame_delay_us = 10000;

	fd = open(ctx->lcd_device, O_WRONLY);
	if (fd < 0) {
		perror("open lcd");
		return NULL;
	}

	while (!atomic_load(&ctx->audio_done)) {
		for (int i = 0; i < frame_count && !atomic_load(&ctx->audio_done); ++i) {
			ssize_t wr = write(fd, frames[i], FRAME_SIZE_BYTES);
			if (wr < 0) {
				perror("write lcd");
				close(fd);
				return NULL;
			}
			{
				struct timespec ts;
				ts.tv_sec = frame_delay_us / 1000000;
				ts.tv_nsec = (long)(frame_delay_us % 1000000) * 1000L;
				nanosleep(&ts, NULL);
			}
		}
	}

	close(fd);
	return NULL;
}

int main(int argc, char **argv)
{
	struct app_ctx ctx;
	pthread_t audio_tid;
	pthread_t image_tid;
	int rc;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <audio.raw> [pcm_device] [lcd_device]\n", argv[0]);
		fprintf(stderr, "Example: %s audio.raw hw:2,0 /dev/ili9225_char\n", argv[0]);
		return 1;
	}

	memset(&ctx, 0, sizeof(ctx));
	ctx.audio_path = argv[1];
	ctx.pcm_device = (argc >= 3) ? argv[2] : "hw:0,0";
	ctx.lcd_device = (argc >= 4) ? argv[3] : "/dev/ili9225_char";
	ctx.audio_seconds = pcm_duration_seconds_from_file(ctx.audio_path);
	atomic_init(&ctx.audio_done, 0);

	rc = pthread_create(&audio_tid, NULL, audio_thread, &ctx);
	if (rc != 0) {
		fprintf(stderr, "pthread_create(audio) failed: %s\n", strerror(rc));
		return 1;
	}

	rc = pthread_create(&image_tid, NULL, image_thread, &ctx);
	if (rc != 0) {
		fprintf(stderr, "pthread_create(image) failed: %s\n", strerror(rc));
		atomic_store(&ctx.audio_done, 1);
		pthread_join(audio_tid, NULL);
		return 1;
	}

	pthread_join(audio_tid, NULL);
	pthread_join(image_tid, NULL);
	return 0;
}
