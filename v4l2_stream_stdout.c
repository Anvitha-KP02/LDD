// v4l2_stream_stdout.c
// Live V4L2 capture (MMAP) -> stdout (raw YUYV frames)
// Usage:
//   ./v4l2_stream_stdout            # uses /dev/video0
//   ./v4l2_stream_stdout /dev/video2
//
// Live view (recommended):
//   ./v4l2_stream_stdout /dev/video0 | ffplay -f rawvideo -pixel_format yuyv422 -video_size 640x480 -framerate 30 -i -

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <time.h>

#include <linux/videodev2.h>

typedef struct {
    void   *start;
    size_t  length;
} Buffer;

static int xioctl(int fd, unsigned long req, void *arg)
{
    int r;
    do { r = ioctl(fd, req, arg); }
    while (r == -1 && errno == EINTR);
    return r;
}

static uint64_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
}

int main(int argc, char **argv)
{
    const char *devnode = "/dev/video0";
    if (argc >= 2) devnode = argv[1];

    const int width = 640;
    const int height = 480;
    const uint32_t pixfmt = V4L2_PIX_FMT_YUYV;

    int fd = open(devnode, O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) { perror("open"); return 1; }

    // Query caps
    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));
    if (xioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        perror("VIDIOC_QUERYCAP"); close(fd); return 1;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
        !(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "Device not capture/streaming capable\n");
        close(fd); return 1;
    }

    // Set format
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = pixfmt;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT"); close(fd); return 1;
    }

    // Request buffers
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (xioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("VIDIOC_REQBUFS"); close(fd); return 1;
    }
    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory\n");
        close(fd); return 1;
    }

    Buffer *bufs = calloc(req.count, sizeof(Buffer));
    if (!bufs) { perror("calloc"); close(fd); return 1; }

    // mmap buffers
    for (unsigned i = 0; i < req.count; i++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = req.type;
        buf.memory = req.memory;
        buf.index = i;

        if (xioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {
            perror("VIDIOC_QUERYBUF"); close(fd); return 1;
        }

        bufs[i].length = buf.length;
        bufs[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE,
                             MAP_SHARED, fd, buf.m.offset);
        if (bufs[i].start == MAP_FAILED) {
            perror("mmap"); close(fd); return 1;
        }
    }

    // Queue all buffers
    for (unsigned i = 0; i < req.count; i++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = req.type;
        buf.memory = req.memory;
        buf.index = i;

        if (xioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            perror("VIDIOC_QBUF"); close(fd); return 1;
        }
    }

    // Stream on
    enum v4l2_buf_type type = req.type;
    if (xioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        perror("VIDIOC_STREAMON"); close(fd); return 1;
    }

    // Make stdout unbuffered (important for piping to ffplay)
    setvbuf(stdout, NULL, _IONBF, 0);

    uint64_t t0 = now_ms();
    uint64_t last_report = t0;
    unsigned frames = 0;

    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        int r = select(fd + 1, &fds, NULL, NULL, &tv);
        if (r == -1) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }
        if (r == 0) {
            fprintf(stderr, "select timeout\n");
            continue;
        }

        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = req.type;
        buf.memory = req.memory;

        if (xioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
            if (errno == EAGAIN) continue;
            perror("VIDIOC_DQBUF");
            break;
        }

        // Write the raw frame to stdout (live stream)
        // NOTE: buf.bytesused is the valid payload.
        if (write(STDOUT_FILENO, bufs[buf.index].start, buf.bytesused) < 0) {
            perror("write(stdout)");
            // If pipe breaks (e.g., ffplay closed), exit cleanly.
            break;
        }

        // Re-queue
        if (xioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            perror("VIDIOC_QBUF");
            break;
        }

        frames++;
        uint64_t now = now_ms();
        if (now - last_report >= 1000) {
            double secs = (now - last_report) / 1000.0;
            double fps = frames / secs;
            fprintf(stderr, "\rLive FPS: %.1f        ", fps);
            frames = 0;
            last_report = now;
        }
    }

    // Stream off (best effort)
    xioctl(fd, VIDIOC_STREAMOFF, &type);

    for (unsigned i = 0; i < req.count; i++) {
        if (bufs[i].start && bufs[i].start != MAP_FAILED)
            munmap(bufs[i].start, bufs[i].length);
    }
    free(bufs);
    close(fd);

    fprintf(stderr, "\nDone.\n");
    return 0;
}
