#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main() {
    int fbfd = open("/dev/fb0", O_RDWR);

    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo);

    int screensize = vinfo.yres_virtual * finfo.line_length;

    unsigned char *fbp = (unsigned char*) mmap(
        0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

    int x = 10, y = 20;
    int bytes_per_pixel = vinfo.bits_per_pixel / 8;

    long location = (x + vinfo.xoffset) * bytes_per_pixel +
                    (y + vinfo.yoffset) * finfo.line_length;

    // Writing RED color (RGB888)
    *(fbp + location) = 0;        // Blue
    *(fbp + location + 1) = 0;    // Green
    *(fbp + location + 2) = 255;  // Red

    munmap(fbp, screensize);
    close(fbfd);

    return 0;
}
