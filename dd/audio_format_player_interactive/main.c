#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PCM_RATE_HZ 44100
#define PCM_CHANNELS 2

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

static int run_execvp(char *const argv[])
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		return -1;
	}
	if (pid == 0) {
		execvp(argv[0], argv);
		perror("execvp");
		_exit(127);
	}

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
		fprintf(stderr, "Command did not exit normally\n");
		return -1;
	}
	if (WEXITSTATUS(status) != 0) {
		fprintf(stderr, "Command failed (exit=%d)\n", WEXITSTATUS(status));
		return -1;
	}
	return 0;
}

static int convert_to_raw_pcm(const char *input_path, const char *output_raw_path)
{
	/*
	 * Decode in user space only:
	 * - output is raw interleaved 16-bit little-endian stereo @ 44100 Hz
	 * - matches kernel driver's fixed hw_params checks
	 */
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
	return run_execvp(ffmpeg_argv);
}

static int play_raw_via_aplay(const char *hw_dev, const char *raw_path)
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
	return run_execvp(aplay_argv);
}

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

	/* If user typed an exact filename (with extension), accept it. */
	if (is_supported_audio_file(query) && file_exists_regular(query)) {
		if (snprintf(out_path, out_sz, "%s", query) < 0)
			return -1;
		return 0;
	}

	/* a) Exact basename match: format1 -> format1.mp3/.aac/.flac (in that order). */
	for (i = 0; i < (sizeof(exts) / sizeof(exts[0])); i++) {
		if (build_exact_basename_candidate(query, exts[i], candidate, sizeof(candidate)) != 0)
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

	/* b) Extension search: ".mp3" -> first *.mp3 in current directory. */
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

	/* c) Substring search: "format" -> first file containing substring and having supported extension. */
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

int main(int argc, char **argv)
{
	int fmt = 0;
	char input_path_buf[PATH_MAX];
	const char *input_path = NULL;
	char raw_template[] = "/tmp/decoded_XXXXXX";
	int fd = -1;
	char hw_dev[64];
	const char *default_hw = getenv("AUDIO_HW");
	char query[PATH_MAX];

	if (argc >= 2 && argv[1] && argv[1][0]) {
		strncpy(hw_dev, argv[1], sizeof(hw_dev) - 1);
		hw_dev[sizeof(hw_dev) - 1] = '\0';
	} else if (default_hw && default_hw[0]) {
		strncpy(hw_dev, default_hw, sizeof(hw_dev) - 1);
		hw_dev[sizeof(hw_dev) - 1] = '\0';
	} else {
		strncpy(hw_dev, "hw:0,0", sizeof(hw_dev) - 1);
		hw_dev[sizeof(hw_dev) - 1] = '\0';
	}

	fprintf(stdout,
		"=== Multi-format Audio Player (user-space decode -> ALSA RAW) ===\n"
		"Kernel-ready PCM: S16_LE, %dch, %dHz | ALSA device: %s\n"
		"Usage: %s [hw:X,Y]\n\n",
		PCM_CHANNELS, PCM_RATE_HZ, hw_dev, argv[0]);

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
			"No matching audio file found in current directory for: %s\n"
			"Try: a basename (format1), an extension (.mp3), or a substring (format)\n",
			query);
		return 1;
	}
	input_path = input_path_buf;
	fmt = detect_format_from_extension(input_path);
	if (fmt == 0) {
		fprintf(stderr, "Unsupported audio format for file: %s\n", input_path);
		return 1;
	}

	fd = mkstemp(raw_template);
	if (fd < 0) {
		perror("mkstemp");
		return 1;
	}
	close(fd);

	fprintf(stdout, "\nSelected format: %s\nInput file: %s\n",
		format_to_string(fmt), input_path);

	if (convert_to_raw_pcm(input_path, raw_template) != 0) {
		fprintf(stderr, "Conversion failed.\n");
		unlink(raw_template);
		return 1;
	}

	if (play_raw_via_aplay(hw_dev, raw_template) != 0) {
		fprintf(stderr, "Playback failed.\n");
		unlink(raw_template);
		return 1;
	}

	unlink(raw_template);
	fprintf(stdout, "\nDone.\n");
	return 0;
}
