#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define WIDTH 640
#define HEIGHT 480
#define BUFFER_COUNT 4

// Try USB headset first, fallback to default
#define PCM_DEVICE_PRIMARY "plughw:3,0"
#define PCM_DEVICE_FALLBACK "default"

// ================= VIDEO STRUCT =================
struct buffer {
    void *start;
    size_t length;
};

struct buffer buffers[BUFFER_COUNT];

// ================= AUDIO THREAD =================
void *audio_thread(void *arg)
{
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    int16_t *buffer;
    FILE *fp;
    int err;

    int rate = 44100, channels = 2, frames = 1024;

    buffer = malloc(frames * channels * sizeof(int16_t));

    // Try primary device
    err = snd_pcm_open(&handle, PCM_DEVICE_PRIMARY,
                       SND_PCM_STREAM_CAPTURE, 0);

    if (err < 0) {
        printf("Primary device failed, switching to default...\n");

        // Fallback
        if ((err = snd_pcm_open(&handle, PCM_DEVICE_FALLBACK,
                                SND_PCM_STREAM_CAPTURE, 0)) < 0) {
            fprintf(stderr, "Audio open failed: %s\n", snd_strerror(err));
            return NULL;
        }
    }

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);
    snd_pcm_hw_params_set_access(handle, params,
        SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params,
        SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
    snd_pcm_hw_params_set_channels(handle, params, channels);
    snd_pcm_hw_params(handle, params);

    snd_pcm_prepare(handle);

    fp = fopen("audio.raw", "wb");

    for (int i = 0; i < 200; i++) {
        err = snd_pcm_readi(handle, buffer, frames);
        if (err < 0)
            snd_pcm_recover(handle, err, 0);

        fwrite(buffer, sizeof(int16_t), frames * channels, fp);
    }

    fclose(fp);
    snd_pcm_close(handle);
    free(buffer);

    return NULL;
}

// ================= VIDEO THREAD =================
void *video_thread(void *arg)
{
    int fd, i;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    FILE *fp;

    fd = open("/dev/video0", O_RDWR);
    if (fd < 0) {
        perror("Camera open failed");
        return NULL;
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

    ioctl(fd, VIDIOC_S_FMT, &fmt);

    memset(&req, 0, sizeof(req));
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    ioctl(fd, VIDIOC_REQBUFS, &req);

    for (i = 0; i < BUFFER_COUNT; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.type = req.type;
        buf.memory = req.memory;
        buf.index = i;

        ioctl(fd, VIDIOC_QUERYBUF, &buf);

        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED, fd,
                                buf.m.offset);
    }

    for (i = 0; i < BUFFER_COUNT; i++) {
        buf.type = req.type;
        buf.memory = req.memory;
        buf.index = i;
        ioctl(fd, VIDIOC_QBUF, &buf);
    }

    int type = req.type;
    ioctl(fd, VIDIOC_STREAMON, &type);

    fp = fopen("video.raw", "wb");

    for (i = 0; i < 200; i++) {
        ioctl(fd, VIDIOC_DQBUF, &buf);

        fwrite(buffers[buf.index].start,
               buf.bytesused, 1, fp);

        ioctl(fd, VIDIOC_QBUF, &buf);
    }

    fclose(fp);
    ioctl(fd, VIDIOC_STREAMOFF, &type);

    for (i = 0; i < BUFFER_COUNT; i++)
        munmap(buffers[i].start, buffers[i].length);

    close(fd);
    return NULL;
}

// ================= MAIN =================
int main()
{
    pthread_t video_tid, audio_tid;

    pthread_create(&video_tid, NULL, video_thread, NULL);
    pthread_create(&audio_tid, NULL, audio_thread, NULL);

    pthread_join(video_tid, NULL);
    pthread_join(audio_tid, NULL);

    printf("Audio + Video capture complete\n");
    return 0;
}
