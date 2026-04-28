/*
 * rpi_raw_player_user.c
 *
 * User-space utility for the demo driver:
 * 1) Convert MP3 -> RAW PCM with ffmpeg.
 * 2) Trigger kernel ioctl (PLAY_RAW / PLAY_MP3).
 * 3) Send RAW PCM samples to ALSA PCM playback device.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>

#define CTRL_NODE            "/dev/rpi_raw_ctl"
#define DEFAULT_PCM_DEVICE   "hw:0,0"
#define CHUNK_FRAMES         1024
#define CHANNELS             2

#define RPI_IOC_MAGIC        'R'
#define PLAY_RAW             _IO(RPI_IOC_MAGIC, 1)
#define PLAY_MP3             _IOW(RPI_IOC_MAGIC, 2, struct rpi_mp3_req)

struct rpi_mp3_req {
	char mp3_path[256];
	char raw_path[256];
};

static int convert_mp3_to_raw(const char *mp3, const char *raw)
{
	char cmd[768];
	int ret;

	snprintf(cmd, sizeof(cmd),
		 "ffmpeg -y -i '%s' -f s16le -acodec pcm_s16le "
		 "-ac 2 -ar 44100 '%s'",
		 mp3, raw);

	printf("[USER] Converting MP3 to RAW:\n%s\n", cmd);
	ret = system(cmd);
	if (ret != 0) {
		fprintf(stderr, "[USER] ffmpeg failed: ret=%d\n", ret);
		return -1;
	}
	return 0;
}

static int arm_playback_raw(void)
{
	int fd;

	fd = open(CTRL_NODE, O_RDWR);
	if (fd < 0) {
		perror("open control node");
		return -1;
	}

	if (ioctl(fd, PLAY_RAW) < 0) {
		perror("ioctl PLAY_RAW");
		close(fd);
		return -1;
	}

	printf("[USER] ioctl PLAY_RAW done\n");
	close(fd);
	return 0;
}

static int arm_playback_mp3(const char *mp3, const char *raw)
{
	int fd;
	struct rpi_mp3_req req;

	memset(&req, 0, sizeof(req));
	snprintf(req.mp3_path, sizeof(req.mp3_path), "%s", mp3);
	snprintf(req.raw_path, sizeof(req.raw_path), "%s", raw);

	fd = open(CTRL_NODE, O_RDWR);
	if (fd < 0) {
		perror("open control node");
		return -1;
	}

	if (ioctl(fd, PLAY_MP3, &req) < 0) {
		perror("ioctl PLAY_MP3");
		close(fd);
		return -1;
	}

	printf("[USER] ioctl PLAY_MP3 done\n");
	close(fd);
	return 0;
}

static int play_raw_via_alsa(const char *pcm_dev, const char *raw_path)
{
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	FILE *fp;
	int16_t *buffer;
	int rc;
	size_t items;

	fp = fopen(raw_path, "rb");
	if (!fp) {
		perror("fopen raw");
		return -1;
	}

	rc = snd_pcm_open(&handle, pcm_dev, SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0) {
		fprintf(stderr, "snd_pcm_open: %s\n", snd_strerror(rc));
		fclose(fp);
		return -1;
	}

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(handle, params, 2);
	{
		unsigned int rate = 44100;
		snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
	}
	snd_pcm_hw_params_set_period_size_near(handle, params,
					       (snd_pcm_uframes_t[]){CHUNK_FRAMES}, 0);

	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0) {
		fprintf(stderr, "snd_pcm_hw_params: %s\n", snd_strerror(rc));
		snd_pcm_close(handle);
		fclose(fp);
		return -1;
	}

	buffer = malloc(CHUNK_FRAMES * CHANNELS * sizeof(int16_t));
	if (!buffer) {
		perror("malloc");
		snd_pcm_close(handle);
		fclose(fp);
		return -1;
	}

	printf("[USER] Playing RAW on ALSA device: %s\n", pcm_dev);
	while ((items = fread(buffer, CHANNELS * sizeof(int16_t), CHUNK_FRAMES, fp)) > 0) {
		rc = snd_pcm_writei(handle, buffer, items);
		if (rc == -EPIPE) {
			snd_pcm_prepare(handle);
		} else if (rc < 0) {
			fprintf(stderr, "snd_pcm_writei: %s\n", snd_strerror(rc));
			break;
		}
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);
	fclose(fp);
	printf("[USER] Playback complete\n");
	return 0;
}

static void usage(const char *prog)
{
	printf("Usage:\n");
	printf("  %s --raw <input.raw> [--pcm hw:0,0]\n", prog);
	printf("  %s --mp3 <input.mp3> --raw <output.raw> [--pcm hw:0,0]\n", prog);
}

int main(int argc, char **argv)
{
	const char *mp3 = NULL;
	const char *raw = NULL;
	const char *pcm = DEFAULT_PCM_DEVICE;
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--mp3") && i + 1 < argc) {
			mp3 = argv[++i];
		} else if (!strcmp(argv[i], "--raw") && i + 1 < argc) {
			raw = argv[++i];
		} else if (!strcmp(argv[i], "--pcm") && i + 1 < argc) {
			pcm = argv[++i];
		} else {
			usage(argv[0]);
			return 1;
		}
	}

	if (!raw) {
		usage(argv[0]);
		return 1;
	}

	if (mp3) {
		if (arm_playback_mp3(mp3, raw) < 0)
			return 1;
		if (convert_mp3_to_raw(mp3, raw) < 0)
			return 1;
	} else {
		if (arm_playback_raw() < 0)
			return 1;
	}

	return play_raw_via_alsa(pcm, raw);
}
