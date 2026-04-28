/*
 * User-space audio (ffmpeg -> raw, aplay -> ALSA) with concurrent SPI LCD
 * updates via /dev/ili9225_char (RGB565 full-screen buffer per write).
 *
 * LCD: ILI9225 176x220, rgb565le, 77440 bytes per frame (matches kernel drawImage).
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define PCM_RATE_HZ 44100
#define PCM_CHANNELS 2

#define LCD_WIDTH 176
#define LCD_HEIGHT 220
#define LCD_FRAME_BYTES ((size_t)(LCD_WIDTH * LCD_HEIGHT * 2))

#define DEFAULT_LCD_DEV "/dev/ili9225_char"
#define DEFAULT_HW "hw:0,0"

/* -------------------------------------------------------------------------- */
/* Small helpers                                                              */
/* -------------------------------------------------------------------------- */

static void trim_newline(char *s)
{
	size_t n;

	if (!s)
		return;
	n = strlen(s);
	if (n && s[n - 1] == '\n')
		s[n - 1] = '\0';
}

static int file_exists_regular(const char *path)
{
	struct stat st;

	if (!path || !path[0])
		return 0;
	if (stat(path, &st) != 0)
		return 0;
	return S_ISREG(st.st_mode);
}

static int read_full_file(const char *path, void *buf, size_t expect)
{
	int fd;
	ssize_t n;
	size_t got;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		perror(path);
		return -1;
	}

	for (got = 0; got < expect;) {
		n = read(fd, (char *)buf + got, expect - got);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			perror("read");
			close(fd);
			return -1;
		}
		if (n == 0)
			break;
		got += (size_t)n;
	}
	close(fd);

	if (got != expect) {
		fprintf(stderr, "%s: expected %zu bytes, got %zu\n", path, expect, got);
		return -1;
	}
	return 0;
}

static int lcd_open(const char *devpath)
{
	int fd = open(devpath, O_WRONLY);

	if (fd < 0)
		perror(devpath);
	return fd;
}

static int lcd_write_frame(int lcd_fd, const void *frame, size_t len)
{
	const char *p = frame;
	size_t sent = 0;
	ssize_t n;

	if (len != LCD_FRAME_BYTES) {
		fprintf(stderr, "LCD frame must be %zu bytes (got %zu)\n",
			LCD_FRAME_BYTES, len);
		return -1;
	}

	while (sent < len) {
		n = write(lcd_fd, p + sent, len - sent);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			perror("write(/dev/ili9225_char)");
			return -1;
		}
		if (n == 0) {
			fprintf(stderr, "write: short write\n");
			return -1;
		}
		sent += (size_t)n;
	}
	return 0;
}

/* -------------------------------------------------------------------------- */
/* ffmpeg / aplay                                                             */
/* -------------------------------------------------------------------------- */

static pid_t spawn_execvp(char *const argv[])
{
	pid_t pid = fork();

	if (pid < 0) {
		perror("fork");
		return (pid_t)-1;
	}
	if (pid == 0) {
		execvp(argv[0], argv);
		perror("execvp");
		_exit(127);
	}
	return pid;
}

static int run_execvp_wait(char *const argv[])
{
	pid_t pid = spawn_execvp(argv);
	int status;

	if (pid < 0)
		return -1;

	for (;;) {
		if (waitpid(pid, &status, 0) < 0) {
			if (errno == EINTR)
				continue;
			perror("waitpid");
			return -1;
		}
		break;
	}

	if (!WIFEXITED(status)) {
		fprintf(stderr, "%s: did not exit normally\n", argv[0]);
		return -1;
	}
	if (WEXITSTATUS(status) != 0) {
		fprintf(stderr, "%s failed (exit=%d)\n", argv[0], WEXITSTATUS(status));
		return -1;
	}
	return 0;
}

static int convert_to_raw_pcm(const char *input_path, const char *output_raw_path)
{
	char rate_arg[32];
	char ch_arg[32];

	snprintf(rate_arg, sizeof(rate_arg), "%d", PCM_RATE_HZ);
	snprintf(ch_arg, sizeof(ch_arg), "%d", PCM_CHANNELS);

	char *const ffmpeg_argv[] = {
		"ffmpeg",
		"-y",
		"-hide_banner",
		"-loglevel",
		"error",
		"-i",
		(char *)input_path,
		"-f",
		"s16le",
		"-acodec",
		"pcm_s16le",
		"-ac",
		ch_arg,
		"-ar",
		rate_arg,
		(char *)output_raw_path,
		NULL,
	};

	fprintf(stdout, "\n[convert] ffmpeg -> %s\n", output_raw_path);
	return run_execvp_wait(ffmpeg_argv);
}

static pid_t play_raw_via_aplay_async(const char *hw_dev, const char *raw_path)
{
	char rate_arg[32];
	char ch_arg[32];

	snprintf(rate_arg, sizeof(rate_arg), "%d", PCM_RATE_HZ);
	snprintf(ch_arg, sizeof(ch_arg), "%d", PCM_CHANNELS);

	char *const aplay_argv[] = {
		"aplay",
		"-D",
		(char *)hw_dev,
		"-t",
		"raw",
		"-f",
		"S16_LE",
		"-c",
		ch_arg,
		"-r",
		rate_arg,
		(char *)raw_path,
		NULL,
	};

	fprintf(stdout, "\n[play] aplay -D %s %s (async)\n", hw_dev, raw_path);
	return spawn_execvp(aplay_argv);
}

/* Blocking variant if you need sequential play without LCD */
static int play_raw_via_aplay_sync(const char *hw_dev, const char *raw_path)
{
	char rate_arg[32];
	char ch_arg[32];

	snprintf(rate_arg, sizeof(rate_arg), "%d", PCM_RATE_HZ);
	snprintf(ch_arg, sizeof(ch_arg), "%d", PCM_CHANNELS);

	char *const aplay_argv[] = {
		"aplay",
		"-D",
		(char *)hw_dev,
		"-t",
		"raw",
		"-f",
		"S16_LE",
		"-c",
		ch_arg,
		"-r",
		rate_arg,
		(char *)raw_path,
		NULL,
	};

	fprintf(stdout, "\n[play] aplay -D %s %s\n", hw_dev, raw_path);
	return run_execvp_wait(aplay_argv);
}

/* -------------------------------------------------------------------------- */
/* Audio file discovery (same behaviour as original interactive player)       */
/* -------------------------------------------------------------------------- */

static int str_endswith_case(const char *s, const char *suffix)
{
	size_t sl, sufl, i;

	if (!s || !suffix)
		return 0;
	sl = strlen(s);
	sufl = strlen(suffix);
	if (sufl == 0 || sufl > sl)
		return 0;

	for (i = 0; i < sufl; i++) {
		char a = s[sl - sufl + i];
		char b = suffix[i];

		if (a >= 'A' && a <= 'Z')
			a = (char)(a - 'A' + 'a');
		if (b >= 'A' && b <= 'Z')
			b = (char)(b - 'A' + 'a');
		if (a != b)
			return 0;
	}
	return 1;
}

static int detect_format_from_extension(const char *path)
{
	if (!path)
		return 0;
	if (str_endswith_case(path, ".mp3"))
		return 1;
	if (str_endswith_case(path, ".aac"))
		return 2;
	if (str_endswith_case(path, ".flac"))
		return 3;
	return 0;
}

static const char *format_to_string(int sel)
{
	switch (sel) {
	case 1:
		return "MP3";
	case 2:
		return "AAC";
	case 3:
		return "FLAC";
	default:
		return "UNKNOWN";
	}
}

static int is_supported_audio_file(const char *name)
{
	return detect_format_from_extension(name) != 0;
}

static int build_exact_basename_candidate(const char *base, const char *ext,
					  char *out, size_t out_sz)
{
	int n;

	if (!base || !base[0] || !ext || !ext[0] || !out || out_sz == 0)
		return -1;
	n = snprintf(out, out_sz, "%s%s", base, ext);
	if (n < 0 || (size_t)n >= out_sz)
		return -1;
	return 0;
}

static int find_audio_file(const char *query, char *out_path, size_t out_sz)
{
	static const char *exts[] = { ".mp3", ".aac", ".flac" };
	char candidate[PATH_MAX];
	DIR *d = NULL;
	struct dirent *de;
	size_t i;

	if (!query || !query[0] || !out_path || out_sz == 0)
		return -1;

	if (is_supported_audio_file(query) && file_exists_regular(query)) {
		if (snprintf(out_path, out_sz, "%s", query) < 0)
			return -1;
		return 0;
	}

	for (i = 0; i < (sizeof(exts) / sizeof(exts[0])); i++) {
		if (build_exact_basename_candidate(query, exts[i], candidate,
						   sizeof(candidate)) != 0)
			continue;
		if (file_exists_regular(candidate)) {
			if (snprintf(out_path, out_sz, "%s", candidate) < 0)
				return -1;
			return 0;
		}
	}

	d = opendir(".");
	if (!d) {
		perror("opendir");
		return -1;
	}

	if (query[0] == '.' && query[1] != '\0') {
		while ((de = readdir(d)) != NULL) {
			const char *name = de->d_name;

			if (!name || !name[0])
				continue;
			if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
				continue;
			if (!str_endswith_case(name, query))
				continue;
			if (!file_exists_regular(name))
				continue;
			if (snprintf(out_path, out_sz, "%s", name) < 0) {
				closedir(d);
				return -1;
			}
			closedir(d);
			return 0;
		}
		closedir(d);
		return -1;
	}

	while ((de = readdir(d)) != NULL) {
		const char *name = de->d_name;

		if (!name || !name[0])
			continue;
		if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
			continue;
		if (!is_supported_audio_file(name))
			continue;
		if (!strstr(name, query))
			continue;
		if (!file_exists_regular(name))
			continue;

		if (snprintf(out_path, out_sz, "%s", name) < 0) {
			closedir(d);
			return -1;
		}
		closedir(d);
		return 0;
	}

	closedir(d);
	return -1;
}

/* -------------------------------------------------------------------------- */
/* Display while audio child runs                                             */
/* -------------------------------------------------------------------------- */

static int display_static_while_playing(int lcd_fd, const char *img_path,
					pid_t aplay_pid)
{
	uint8_t *frame = malloc(LCD_FRAME_BYTES);

	if (!frame)
		return -1;

	if (read_full_file(img_path, frame, LCD_FRAME_BYTES) != 0) {
		free(frame);
		kill(aplay_pid, SIGTERM);
		waitpid(aplay_pid, NULL, 0);
		return -1;
	}

	if (lcd_write_frame(lcd_fd, frame, LCD_FRAME_BYTES) != 0) {
		free(frame);
		kill(aplay_pid, SIGTERM);
		waitpid(aplay_pid, NULL, 0);
		return -1;
	}
	free(frame);

	/* Hold until playback process exits */
	for (;;) {
		int st;
		pid_t w = waitpid(aplay_pid, &st, 0);

		if (w < 0) {
			if (errno == EINTR)
				continue;
			perror("waitpid");
			return -1;
		}
		break;
	}
	return 0;
}

static int cmp_str(const void *a, const void *b)
{
	return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static int collect_raw_files(const char *dir, char ***out_names, size_t *out_n)
{
	DIR *d;
	struct dirent *de;
	char **list = NULL;
	size_t n = 0, cap = 0;
	char path[PATH_MAX];

	d = opendir(dir);
	if (!d) {
		perror(dir);
		return -1;
	}

	while ((de = readdir(d)) != NULL) {
		const char *name = de->d_name;

		if (!name || !str_endswith_case(name, ".raw"))
			continue;
		if (snprintf(path, sizeof(path), "%s/%s", dir, name) >= (int)sizeof(path)) {
			closedir(d);
			free(list);
			return -1;
		}
		if (!file_exists_regular(path))
			continue;

		if (n == cap) {
			size_t ncap = cap ? cap * 2 : 16;
			char **nl = realloc(list, ncap * sizeof(*nl));

			if (!nl) {
				closedir(d);
				free(list);
				return -1;
			}
			list = nl;
			cap = ncap;
		}
		list[n] = strdup(name);
		if (!list[n]) {
			closedir(d);
			while (n)
				free(list[--n]);
			free(list);
			return -1;
		}
		n++;
	}
	closedir(d);

	if (n == 0) {
		fprintf(stderr, "No .raw files in %s\n", dir);
		free(list);
		return -1;
	}

	qsort(list, n, sizeof(*list), cmp_str);
	*out_names = list;
	*out_n = n;
	return 0;
}

static int display_animation_while_playing(int lcd_fd, const char *dir,
					   double fps, pid_t aplay_pid)
{
	char **names = NULL;
	size_t nfiles = 0;
	uint8_t *frame = malloc(LCD_FRAME_BYTES);
	size_t i = 0;
	struct timespec ts;
	long ns_per_frame;

	if (!frame)
		return -1;

	if (fps <= 0.0)
		fps = 10.0;
	ns_per_frame = (long)(1000000000.0 / fps);

	if (collect_raw_files(dir, &names, &nfiles) != 0) {
		free(frame);
		return -1;
	}

	ts.tv_sec = 0;
	ts.tv_nsec = ns_per_frame;

	for (;;) {
		int st;
		pid_t w = waitpid(aplay_pid, &st, WNOHANG);

		if (w < 0) {
			if (errno == EINTR)
				continue;
			perror("waitpid");
			free(frame);
			for (i = 0; i < nfiles; i++)
				free(names[i]);
			free(names);
			return -1;
		}
		if (w == aplay_pid)
			break;

		{
			char path[PATH_MAX];

			if (snprintf(path, sizeof(path), "%s/%s", dir, names[i % nfiles]) >= (int)sizeof(path)) {
				kill(aplay_pid, SIGTERM);
				waitpid(aplay_pid, NULL, 0);
				free(frame);
				for (i = 0; i < nfiles; i++)
					free(names[i]);
				free(names);
				return -1;
			}

			if (read_full_file(path, frame, LCD_FRAME_BYTES) == 0)
				(void)lcd_write_frame(lcd_fd, frame, LCD_FRAME_BYTES);
		}

		i++;
		nanosleep(&ts, NULL);
	}

	free(frame);
	for (i = 0; i < nfiles; i++)
		free(names[i]);
	free(names);
	return 0;
}

struct thread_arg {
	int lcd_fd;
	const char *image_path;
	const char *anim_dir;
	double fps;
	pid_t aplay_pid;
	int err;
};

static void *display_thread_main(void *p)
{
	struct thread_arg *a = p;

	if (a->anim_dir && a->anim_dir[0])
		a->err = display_animation_while_playing(a->lcd_fd, a->anim_dir,
							 a->fps, a->aplay_pid);
	else if (a->image_path && a->image_path[0])
		a->err = display_static_while_playing(a->lcd_fd, a->image_path,
						      a->aplay_pid);
	else {
		/* No LCD content: just wait for aplay */
		for (;;) {
			int st;
			pid_t w = waitpid(a->aplay_pid, &st, 0);

			if (w < 0) {
				if (errno == EINTR)
					continue;
				perror("waitpid");
				a->err = -1;
				return NULL;
			}
			break;
		}
		a->err = 0;
	}
	return NULL;
}

static void usage(const char *prog)
{
	fprintf(stderr,
		"Usage: %s [hw:X,Y] [options]\n"
		"  --lcd PATH          LCD device (default %s or $LCD_DEV)\n"
		"  --image FILE        single RGB565 raw frame (%dx%d, %zu bytes)\n"
		"  --anim-dir DIR      cycle all *.raw in DIR sorted by name\n"
		"  --fps N             animation frame rate (default 10)\n"
		"  --sync-aplay        use blocking aplay (no concurrent LCD)\n"
		"\n"
		"Environment: AUDIO_HW=hw:x,y  LCD_DEV=/dev/ili9225_char\n",
		prog, DEFAULT_LCD_DEV, LCD_WIDTH, LCD_HEIGHT, LCD_FRAME_BYTES);
}

int main(int argc, char **argv)
{
	char input_path_buf[PATH_MAX];
	const char *input_path = NULL;
	char raw_template[] = "/tmp/decoded_XXXXXX";
	int raw_fd = -1;
	char hw_dev[64];
	const char *audio_hw_env = getenv("AUDIO_HW");
	char query[PATH_MAX];
	const char *lcd_env = getenv("LCD_DEV");
	const char *lcd_dev;
	const char *image_path = NULL;
	const char *anim_dir = NULL;
	double fps = 10.0;
	int argi = 1;
	bool sync_aplay = false;
	pthread_t disp_thr;
	struct thread_arg targ;
	int lcd_fd = -1;

	lcd_dev = (lcd_env && lcd_env[0]) ? lcd_env : DEFAULT_LCD_DEV;

	strncpy(hw_dev, audio_hw_env && audio_hw_env[0] ? audio_hw_env : DEFAULT_HW,
		 sizeof(hw_dev) - 1);
	hw_dev[sizeof(hw_dev) - 1] = '\0';

	for (argi = 1; argi < argc; argi++) {
		const char *a = argv[argi];

		if (strcmp(a, "--lcd") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "--lcd needs a path\n");
				return 1;
			}
			lcd_dev = argv[++argi];
		} else if (strcmp(a, "--image") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "--image needs a file\n");
				return 1;
			}
			image_path = argv[++argi];
		} else if (strcmp(a, "--anim-dir") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "--anim-dir needs a directory\n");
				return 1;
			}
			anim_dir = argv[++argi];
		} else if (strcmp(a, "--fps") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "--fps needs a number\n");
				return 1;
			}
			fps = strtod(argv[++argi], NULL);
		} else if (strcmp(a, "--sync-aplay") == 0) {
			sync_aplay = true;
		} else if (strcmp(a, "--help") == 0 || strcmp(a, "-h") == 0) {
			usage(argv[0]);
			return 0;
		} else if (a[0] == '-') {
			fprintf(stderr, "Unknown option: %s\n", a);
			usage(argv[0]);
			return 1;
		} else {
			/* First non-option: ALSA device string (e.g. hw:0,0) */
			strncpy(hw_dev, a, sizeof(hw_dev) - 1);
			hw_dev[sizeof(hw_dev) - 1] = '\0';
		}
	}

	fprintf(stdout,
		"=== Audio + ILI9225 (user decode -> ALSA, LCD via write) ===\n"
		"PCM: S16_LE, %dch, %dHz | ALSA: %s | LCD: %s\n\n",
		PCM_CHANNELS, PCM_RATE_HZ, hw_dev, lcd_dev);

	fprintf(stdout, "What do you want to play? ");
	fflush(stdout);
	if (!fgets(query, sizeof(query), stdin)) {
		fprintf(stderr, "Failed to read selection (EOF or input error).\n");
		return 1;
	}
	trim_newline(query);
	if (!query[0]) {
		fprintf(stderr, "Input cannot be empty.\n");
		return 1;
	}

	if (find_audio_file(query, input_path_buf, sizeof(input_path_buf)) != 0) {
		fprintf(stderr,
			"No matching audio file for: %s\n"
			"Try: basename, .mp3, or substring\n",
			query);
		return 1;
	}
	input_path = input_path_buf;

	if (detect_format_from_extension(input_path) == 0) {
		fprintf(stderr, "Unsupported audio format: %s\n", input_path);
		return 1;
	}

	fprintf(stdout, "\nSelected: %s | file: %s\n",
		format_to_string(detect_format_from_extension(input_path)),
		input_path);

	raw_fd = mkstemp(raw_template);
	if (raw_fd < 0) {
		perror("mkstemp");
		return 1;
	}
	close(raw_fd);

	if (convert_to_raw_pcm(input_path, raw_template) != 0) {
		fprintf(stderr, "Conversion failed.\n");
		unlink(raw_template);
		return 1;
	}

	if (sync_aplay) {
		int r = play_raw_via_aplay_sync(hw_dev, raw_template);

		unlink(raw_template);
		return r != 0 ? 1 : 0;
	}

	if ((image_path && image_path[0]) || (anim_dir && anim_dir[0])) {
		lcd_fd = lcd_open(lcd_dev);
		if (lcd_fd < 0) {
			unlink(raw_template);
			return 1;
		}
	}

	{
		pid_t apid = play_raw_via_aplay_async(hw_dev, raw_template);

		if (apid < 0) {
			if (lcd_fd >= 0)
				close(lcd_fd);
			unlink(raw_template);
			return 1;
		}

		memset(&targ, 0, sizeof(targ));
		targ.lcd_fd = lcd_fd;
		targ.image_path = image_path;
		targ.anim_dir = anim_dir;
		targ.fps = fps;
		targ.aplay_pid = apid;
		targ.err = 0;

		if (lcd_fd >= 0 &&
		    ((image_path && image_path[0]) || (anim_dir && anim_dir[0]))) {
			if (pthread_create(&disp_thr, NULL, display_thread_main, &targ) != 0) {
				perror("pthread_create");
				kill(apid, SIGTERM);
				waitpid(apid, NULL, 0);
				close(lcd_fd);
				unlink(raw_template);
				return 1;
			}
			pthread_join(disp_thr, NULL);
			if (targ.err != 0) {
				kill(apid, SIGTERM);
				waitpid(apid, NULL, 0);
				close(lcd_fd);
				unlink(raw_template);
				return 1;
			}
		} else {
			/* No LCD assets: wait for aplay only */
			for (;;) {
				int st;

				if (waitpid(apid, &st, 0) < 0) {
					if (errno == EINTR)
						continue;
					perror("waitpid");
					break;
				}
				break;
			}
		}
	}

	if (lcd_fd >= 0)
		close(lcd_fd);
	unlink(raw_template);
	fprintf(stdout, "\nDone.\n");
	return 0;
}
