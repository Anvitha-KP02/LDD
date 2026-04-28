#include <alsa/asoundlib.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct wav_chunk_header {
	char id[4];
	uint32_t size;
};

struct config {
	const char *device;
	const char *wav_path;
	const char *raw_path;
	unsigned int rate;
	unsigned int channels;
	snd_pcm_format_t format;
};

static void usage(const char *prog)
{
	printf("Usage:\n");
	printf("  %s --wav <file.wav> [--device hw:0,0]\n", prog);
	printf("  %s --raw <file.raw> --rate <hz> --channels <n> --format <s16_le|s24_le|s32_le|u8> [--device hw:0,0]\n", prog);
}

static snd_pcm_format_t parse_format(const char *s)
{
	if (!strcmp(s, "s16_le"))
		return SND_PCM_FORMAT_S16_LE;
	if (!strcmp(s, "s24_le"))
		return SND_PCM_FORMAT_S24_LE;
	if (!strcmp(s, "s32_le"))
		return SND_PCM_FORMAT_S32_LE;
	if (!strcmp(s, "u8"))
		return SND_PCM_FORMAT_U8;
	return SND_PCM_FORMAT_UNKNOWN;
}

static int setup_pcm(snd_pcm_t **handle, const struct config *cfg)
{
	snd_pcm_hw_params_t *params;
	int rc;

	rc = snd_pcm_open(handle, cfg->device, SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0) {
		fprintf(stderr, "snd_pcm_open failed: %s\n", snd_strerror(rc));
		return -1;
	}

	snd_pcm_hw_params_alloca(&params);
	rc = snd_pcm_hw_params_any(*handle, params);
	if (rc < 0)
		goto err;
	rc = snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0)
		goto err;
	rc = snd_pcm_hw_params_set_format(*handle, params, cfg->format);
	if (rc < 0)
		goto err;
	rc = snd_pcm_hw_params_set_channels(*handle, params, cfg->channels);
	if (rc < 0)
		goto err;

	{
		unsigned int rate = cfg->rate;
		int dir = 0;
		rc = snd_pcm_hw_params_set_rate_near(*handle, params, &rate, &dir);
		if (rc < 0)
			goto err;
	}

	rc = snd_pcm_hw_params(*handle, params);
	if (rc < 0)
		goto err;

	return 0;

err:
	fprintf(stderr, "snd_pcm_hw_params setup failed: %s\n", snd_strerror(rc));
	snd_pcm_close(*handle);
	*handle = NULL;
	return -1;
}

static int play_stream(FILE *fp, const struct config *cfg)
{
	snd_pcm_t *handle = NULL;
	size_t frame_bytes;
	size_t frames_per_chunk = 1024;
	size_t bytes_per_chunk;
	uint8_t *buf;
	int rc = -1;

	frame_bytes = (snd_pcm_format_physical_width(cfg->format) / 8U) * cfg->channels;
	if (frame_bytes == 0) {
		fprintf(stderr, "invalid frame size\n");
		return -1;
	}

	if (setup_pcm(&handle, cfg) < 0)
		return -1;

	bytes_per_chunk = frames_per_chunk * frame_bytes;
	buf = malloc(bytes_per_chunk);
	if (!buf) {
		fprintf(stderr, "malloc failed\n");
		goto out;
	}

	while (1) {
		size_t n = fread(buf, 1, bytes_per_chunk, fp);
		if (n == 0)
			break;

		snd_pcm_sframes_t frames = (snd_pcm_sframes_t)(n / frame_bytes);
		size_t offset_frames = 0;
		while ((snd_pcm_sframes_t)offset_frames < frames) {
			snd_pcm_sframes_t w = snd_pcm_writei(handle, buf + offset_frames * frame_bytes,
							     frames - (snd_pcm_sframes_t)offset_frames);
			if (w == -EPIPE) {
				snd_pcm_prepare(handle);
				continue;
			}
			if (w < 0) {
				fprintf(stderr, "snd_pcm_writei failed: %s\n", snd_strerror((int)w));
				goto free_buf;
			}
			offset_frames += (size_t)w;
		}
	}

	snd_pcm_drain(handle);
	rc = 0;

free_buf:
	free(buf);
out:
	if (handle)
		snd_pcm_close(handle);
	return rc;
}

static int play_wav(const char *path, struct config *cfg)
{
	FILE *fp = fopen(path, "rb");
	char riff[4];
	uint32_t riff_size;
	char wave[4];
	struct wav_chunk_header ch;
	uint16_t audio_format = 0;
	uint16_t bits_per_sample = 0;
	int got_fmt = 0;
	int got_data = 0;
	int rc;

	if (!fp) {
		perror("fopen wav");
		return -1;
	}

	if (fread(riff, 1, 4, fp) != 4 ||
	    fread(&riff_size, 1, sizeof(riff_size), fp) != sizeof(riff_size) ||
	    fread(wave, 1, 4, fp) != 4) {
		fprintf(stderr, "invalid wav header\n");
		fclose(fp);
		return -1;
	}

	(void)riff_size;
	if (memcmp(riff, "RIFF", 4) || memcmp(wave, "WAVE", 4)) {
		fprintf(stderr, "unsupported wav structure\n");
		fclose(fp);
		return -1;
	}

	while (fread(&ch, 1, sizeof(ch), fp) == sizeof(ch)) {
		long next_pos;
		if (!memcmp(ch.id, "fmt ", 4)) {
			uint16_t channels;
			uint32_t sample_rate;
			uint32_t byte_rate;
			uint16_t block_align;

			if (ch.size < 16) {
				fprintf(stderr, "invalid fmt chunk\n");
				fclose(fp);
				return -1;
			}

			if (fread(&audio_format, 1, sizeof(audio_format), fp) != sizeof(audio_format) ||
			    fread(&channels, 1, sizeof(channels), fp) != sizeof(channels) ||
			    fread(&sample_rate, 1, sizeof(sample_rate), fp) != sizeof(sample_rate) ||
			    fread(&byte_rate, 1, sizeof(byte_rate), fp) != sizeof(byte_rate) ||
			    fread(&block_align, 1, sizeof(block_align), fp) != sizeof(block_align) ||
			    fread(&bits_per_sample, 1, sizeof(bits_per_sample), fp) != sizeof(bits_per_sample)) {
				fprintf(stderr, "failed to read fmt chunk\n");
				fclose(fp);
				return -1;
			}

			(void)byte_rate;
			(void)block_align;
			cfg->channels = channels;
			cfg->rate = sample_rate;
			got_fmt = 1;
			next_pos = ftell(fp) + (long)ch.size - 16;
		} else if (!memcmp(ch.id, "data", 4)) {
			got_data = 1;
			break;
		} else {
			next_pos = ftell(fp) + (long)ch.size;
		}

		if (ch.size & 1)
			next_pos++;
		if (fseek(fp, next_pos, SEEK_SET) != 0) {
			fprintf(stderr, "wav chunk seek failed\n");
			fclose(fp);
			return -1;
		}
	}

	if (!got_fmt || !got_data) {
		fprintf(stderr, "unsupported wav structure (missing fmt/data)\n");
		fclose(fp);
		return -1;
	}

	if (audio_format != 1) {
		fprintf(stderr, "only PCM wav is supported\n");
		fclose(fp);
		return -1;
	}

	switch (bits_per_sample) {
	case 8:
		cfg->format = SND_PCM_FORMAT_U8;
		break;
	case 16:
		cfg->format = SND_PCM_FORMAT_S16_LE;
		break;
	case 24:
		cfg->format = SND_PCM_FORMAT_S24_LE;
		break;
	case 32:
		cfg->format = SND_PCM_FORMAT_S32_LE;
		break;
	default:
		fprintf(stderr, "unsupported bits_per_sample: %u\n", bits_per_sample);
		fclose(fp);
		return -1;
	}

	printf("Playing WAV: %s (rate=%u, ch=%u, bits=%u) on %s\n",
	       path, cfg->rate, cfg->channels, bits_per_sample, cfg->device);
	rc = play_stream(fp, cfg);
	fclose(fp);
	return rc;
}

int main(int argc, char **argv)
{
	struct config cfg = {
		.device = "hw:0,0",
		.wav_path = NULL,
		.raw_path = NULL,
		.rate = 44100,
		.channels = 2,
		.format = SND_PCM_FORMAT_S16_LE,
	};
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--device") && i + 1 < argc) {
			cfg.device = argv[++i];
		} else if (!strcmp(argv[i], "--wav") && i + 1 < argc) {
			cfg.wav_path = argv[++i];
		} else if (!strcmp(argv[i], "--raw") && i + 1 < argc) {
			cfg.raw_path = argv[++i];
		} else if (!strcmp(argv[i], "--rate") && i + 1 < argc) {
			cfg.rate = (unsigned int)atoi(argv[++i]);
		} else if (!strcmp(argv[i], "--channels") && i + 1 < argc) {
			cfg.channels = (unsigned int)atoi(argv[++i]);
		} else if (!strcmp(argv[i], "--format") && i + 1 < argc) {
			cfg.format = parse_format(argv[++i]);
		} else {
			usage(argv[0]);
			return 1;
		}
	}

	if (cfg.wav_path && cfg.raw_path) {
		fprintf(stderr, "choose either --wav or --raw\n");
		return 1;
	}

	if (cfg.wav_path)
		return play_wav(cfg.wav_path, &cfg) == 0 ? 0 : 1;

	if (cfg.raw_path) {
		FILE *fp;
		if (cfg.format == SND_PCM_FORMAT_UNKNOWN) {
			fprintf(stderr, "invalid --format\n");
			return 1;
		}
		fp = fopen(cfg.raw_path, "rb");
		if (!fp) {
			perror("fopen raw");
			return 1;
		}
		printf("Playing RAW: %s (rate=%u, ch=%u, fmt=%s) on %s\n",
		       cfg.raw_path, cfg.rate, cfg.channels,
		       snd_pcm_format_name(cfg.format), cfg.device);
		{
			int rc = play_stream(fp, &cfg);
			fclose(fp);
			return rc == 0 ? 0 : 1;
		}
	}

	usage(argv[0]);
	return 1;
}
