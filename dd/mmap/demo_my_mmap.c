#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/*
 * my_mmap() (learning version):
 * - "maps" a file by allocating heap memory (malloc) and reading file bytes.
 * - This is NOT real mmap: no kernel VMAs, no lazy page faults, no shared mapping.
 *
 * Contract:
 * - If size == 0: read the whole file size from stat()
 * - If size  > 0: read exactly 'size' bytes from the start of the file
 * - On success: returns a pointer to a heap buffer of length 'size'
 * - On failure: returns NULL and sets errno
 */
void *my_mmap(const char *filename, size_t size)
{
	int fd = -1;
	struct stat st;
	uint8_t *buf = NULL;
	size_t to_read = 0;
	size_t off = 0;

	if (!filename) {
		errno = EINVAL;
		return NULL;
	}

	fd = open(filename, O_RDONLY);
	if (fd < 0)
		return NULL;

	if (fstat(fd, &st) < 0) {
		close(fd);
		return NULL;
	}

	if (size == 0) {
		if (!S_ISREG(st.st_mode)) {
			/* For non-regular files, "whole file size" is not well-defined. */
			close(fd);
			errno = EINVAL;
			return NULL;
		}
		if (st.st_size < 0) {
			close(fd);
			errno = EINVAL;
			return NULL;
		}
		to_read = (size_t)st.st_size;
	} else {
		to_read = size;
	}

	buf = (uint8_t *)malloc(to_read ? to_read : 1); /* malloc(0) is implementation-defined */
	if (!buf) {
		close(fd);
		errno = ENOMEM;
		return NULL;
	}

	while (off < to_read) {
		ssize_t n = read(fd, buf + off, to_read - off);
		if (n == 0) {
			/* EOF before we read the requested number of bytes */
			free(buf);
			close(fd);
			errno = EIO;
			return NULL;
		}
		if (n < 0) {
			if (errno == EINTR)
				continue;
			free(buf);
			close(fd);
			return NULL;
		}
		off += (size_t)n;
	}

	close(fd);
	return buf;
}

/*
 * my_munmap() (learning version):
 * - Real munmap() removes VM mappings in the kernel and updates page tables.
 * - Here we only free heap memory.
 */
int my_munmap(void *addr)
{
	if (!addr) {
		errno = EINVAL;
		return -1;
	}
	free(addr);
	return 0;
}

static void hexdump_prefix(const void *p, size_t n, size_t max)
{
	const unsigned char *b = (const unsigned char *)p;
	size_t i, lim = n < max ? n : max;
	for (i = 0; i < lim; i++)
		printf("%02x ", b[i]);
	if (n > max)
		printf("... (%zu bytes total)", n);
	putchar('\n');
}

int main(int argc, char **argv)
{
	const char *path;
	size_t size = 0;
	void *p;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <file> [size]\n", argv[0]);
		return 2;
	}

	path = argv[1];
	if (argc >= 3) {
		char *end = NULL;
		unsigned long long v = strtoull(argv[2], &end, 0);
		if (!end || *end != '\0') {
			fprintf(stderr, "Invalid size: '%s'\n", argv[2]);
			return 2;
		}
		size = (size_t)v;
	}

	p = my_mmap(path, size);
	if (!p) {
		fprintf(stderr, "my_mmap failed: %s\n", strerror(errno));
		return 1;
	}

	/* Show the first few bytes to prove we loaded it. */
	if (size == 0) {
		struct stat st;
		int fd = open(path, O_RDONLY);
		if (fd >= 0 && fstat(fd, &st) == 0 && st.st_size > 0) {
			size = (size_t)st.st_size;
		}
		if (fd >= 0)
			close(fd);
	}

	printf("Loaded %zu bytes into heap buffer at %p\n", size, p);
	printf("Hex prefix: ");
	hexdump_prefix(p, size, 32);

	if (my_munmap(p) < 0) {
		fprintf(stderr, "my_munmap failed: %s\n", strerror(errno));
		return 1;
	}

	return 0;
}
