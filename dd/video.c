#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h> // provides v4l2 structures and ioctl commands for communicating with the camera driver

#define WIDTH 640
#define HEIGHT 480   // this both sets the camera capture resolution to 640x480 pixels
#define BUFFER_COUNT 4

struct buffer {
    void *start;   // pointer to mapped memory
    size_t length;  // buffer size
};

struct buffer buffers[BUFFER_COUNT];   // stores the video frame captured from the camera

int main()
{
    int fd;
    struct v4l2_capability cap;    //contains info about driver, card,bus info,version, capabilities,device_caps. From this we will get the information about cap.driver and cap.card
    struct v4l2_format fmt;  // contains type and the union inside that, which contains info about pixels which contains width, height, pixel format, field.
    struct v4l2_requestbuffers req;   // This tells how many buffers i need to allocate
    struct v4l2_buffer buf;  // this structure communicates with the driver about buffers
    int i;

    fd = open("/dev/video0", O_RDWR);    // opens the camera device file, 1st camera device

    if (fd < 0) {
        perror("open");
        return -1;
    }

    /* Query device capabilities */

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {    // This will checks if the device supports video capture.
        perror("VIDIOC_QUERYCAP");
        return -1;
    }

    printf("Driver: %s\n", cap.driver);  // tells which kernel driver is used(ex. uvcvideo)
    printf("Card: %s\n", cap.card);  // device name

    /* Set video format */

    memset(&fmt, 0, sizeof(fmt));

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;     // Here prgm configures resolution, 640x480
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;   // pixel format YUYV
    fmt.fmt.pix.field = V4L2_FIELD_NONE;   // Field : progressive



/*  YUYV format means Y-luminance(brightness), U/V-chrominance(color) , This is a YUV 4:2:2 format*/


    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("VIDIOC_S_FMT");
        return -1;
    }
/* This (ioctl(fd, VIDIOC_S_FMT, &fmt)  line will send the previously filled fmt structure info into the driver using VIDIOC_S_FMT, it will configure that whether the resolution is supported, pixel format is supported, sets the internal buffer.


    /* Request buffers */

    memset(&req, 0, sizeof(req));

    req.count = BUFFER_COUNT;  // requesting for 4 buffers, why 4? bcz camera continously produces frames
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  // This is capturing the video
    req.memory = V4L2_MEMORY_MMAP;  // These buffers are used for video frame storage, allocate buffers in kernel and map into the userspace. 

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {  // VIDIOC_REQBUFS-> request the driver to allocate the buffers in kernel memory
        perror("VIDIOC_REQBUFS");
        return -1;
    }

    /* Map buffers */

    for (i = 0; i < BUFFER_COUNT; i++) {  // it will map the buffer

        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;   // This buffer is used to capture the video frames, which will map to req.type
        buf.memory = V4L2_MEMORY_MMAP;  // This buffer uses memory mapping, which will match to req.memory
        buf.index = i;   // Giving the details of buffer number i=0,1,2,3

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) {   //  give the details of buffer index i, the driver will return sizeof buffer, offset in kernel memroy
            perror("VIDIOC_QUERYBUF");
            return -1;
        }

        buffers[i].length = buf.length;  // storing buffer size(sizeof each buffer), for later use

        buffers[i].start = mmap(NULL,
                                buf.length,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED,
                                fd,
                                buf.m.offset);   // maps the kernel buffer memory into user space

/* NULL --> OS choose addr automatically
buf.length --> size of memory to map
PROT_READ | PROT_WRITE,  -->  read from buffer, write to buffer
MAP_SHARED  --> shared between user space <-> kernel space
buf.m.offset  -->  offset of buffer provided by VIDIOC_QUERYBUF 


Before mmap:

Kernel Space:
---------------------------------
Buffer0 | Buffer1 | Buffer2 | Buffer3
---------------------------------

User Space:
(no access)

After mmap:

User Space:
---------------------------------
buffers[0].start → Buffer0
buffers[1].start → Buffer1
buffers[2].start → Buffer2
buffers[3].start → Buffer3
---------------------------------

👉 Now your program can directly access frame data!
*/

        if (buffers[i].start == MAP_FAILED) {  // checks the mmap success
            perror("mmap");
            return -1;
        }
    }

    /* Queue buffers */

    for (i = 0; i < BUFFER_COUNT; i++) {

        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            perror("VIDIOC_QBUF");
            return -1;
        }
    }

    /* Start streaming */

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        perror("VIDIOC_STREAMON");
        return -1;
    }

    FILE *file = fopen("frame.raw", "wb");

    /* Capture 100 frames */

    for (i = 0; i < 100; i++) {

        memset(&buf, 0, sizeof(buf));

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
            perror("VIDIOC_DQBUF");
            return -1;
        }

        fwrite(buffers[buf.index].start,
               buf.bytesused,
               1,
               file);

        if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
            perror("VIDIOC_QBUF");
            return -1;
        }

        printf("Captured frame %d\n", i);
    }

    fclose(file);

    /* Stop streaming */

    ioctl(fd, VIDIOC_STREAMOFF, &type);

    /* Unmap buffers */

    for (i = 0; i < BUFFER_COUNT; i++)
        munmap(buffers[i].start, buffers[i].length);

    close(fd);

    return 0;
}
