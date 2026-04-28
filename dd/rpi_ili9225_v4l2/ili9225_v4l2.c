// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ILI9225 SPI panel as V4L2 VIDEO_OUTPUT (176x220 RGB565).
 * Device Tree: compatible "ilitek,ili9225"; dc-gpios (or rs-gpios); reset-gpios;
 * optional backlight-gpios; spi0 child.
 *
 * Userspace (any container ffmpeg understands):
 *   ffmpeg -i video.mkv -pix_fmt rgb565le -f v4l2 /dev/videoN
 *
 * Do NOT load the DRM tiny ili9225 driver at the same time (same SPI device).
 * Disable CONFIG_TINYDRM_ILI9225 or blacklist module "ili9225".
 *
 * Init sequence derived from drivers/gpu/drm/tiny/ili9225.c (David Lechner).
 */

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>

#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-vmalloc.h>

#define ILI9225_WRITE_DATA_TO_GRAM	0x22
#define ILI9225_POWER_CONTROL_1	0x10
#define ILI9225_POWER_CONTROL_2	0x11
#define ILI9225_POWER_CONTROL_3	0x12
#define ILI9225_POWER_CONTROL_4	0x13
#define ILI9225_POWER_CONTROL_5	0x14
#define ILI9225_DRIVER_OUTPUT_CONTROL	0x01
#define ILI9225_LCD_AC_DRIVING_CONTROL	0x02
#define ILI9225_ENTRY_MODE		0x03
#define ILI9225_DISPLAY_CONTROL_1	0x07
#define ILI9225_BLANK_PERIOD_CONTROL_1	0x08
#define ILI9225_FRAME_CYCLE_CONTROL	0x0b
#define ILI9225_INTERFACE_CONTROL	0x0c
#define ILI9225_OSCILLATION_CONTROL	0x0f
#define ILI9225_VCI_RECYCLING		0x15
#define ILI9225_RAM_ADDRESS_SET_1	0x20
#define ILI9225_RAM_ADDRESS_SET_2	0x21
#define ILI9225_HORIZ_WINDOW_ADDR_1	0x36
#define ILI9225_HORIZ_WINDOW_ADDR_2	0x37
#define ILI9225_VERT_WINDOW_ADDR_1	0x38
#define ILI9225_VERT_WINDOW_ADDR_2	0x39
#define ILI9225_GATE_SCAN_CONTROL	0x30
#define ILI9225_VERTICAL_SCROLL_1	0x31
#define ILI9225_VERTICAL_SCROLL_2	0x32
#define ILI9225_VERTICAL_SCROLL_3	0x33
#define ILI9225_PARTIAL_DRIVING_POS_1	0x34
#define ILI9225_PARTIAL_DRIVING_POS_2	0x35
#define ILI9225_GAMMA_CONTROL_1		0x50
#define ILI9225_GAMMA_CONTROL_2		0x51
#define ILI9225_GAMMA_CONTROL_3		0x52
#define ILI9225_GAMMA_CONTROL_4		0x53
#define ILI9225_GAMMA_CONTROL_5		0x54
#define ILI9225_GAMMA_CONTROL_6		0x55
#define ILI9225_GAMMA_CONTROL_7		0x56
#define ILI9225_GAMMA_CONTROL_8		0x57
#define ILI9225_GAMMA_CONTROL_9		0x58
#define ILI9225_GAMMA_CONTROL_10	0x59

#define WIDTH	176
#define HEIGHT	220
#define FRAME_SZ	((unsigned int)(WIDTH * HEIGHT * 2))

struct ili9225_buf {
	struct vb2_v4l2_buffer vb;
	struct list_head list;
};

struct ili9225 {
	struct spi_device *spi;
	struct gpio_desc *dc;
	struct gpio_desc *rst;
	struct gpio_desc *backlight;
	struct v4l2_device v4l2_dev;
	struct video_device vdev;
	struct vb2_queue queue;
	struct mutex lock;
	spinlock_t slock;
	struct list_head buf_list;
	struct work_struct work;
	bool streaming;
	u32 rotation;
	bool swap_bytes;
};

/* --- SPI (aligned with drm/tiny/ili9225 ili9225_dbi_command) --- */

static int ili9225_spi_xmit(struct spi_device *spi, u8 bpw, const void *buf,
			    size_t len)
{
	size_t max_chunk = spi_max_transfer_size(spi);
	struct spi_transfer tr = {
		.bits_per_word = bpw,
	};
	struct spi_message m;
	size_t chunk;
	int ret;

	max_chunk = ALIGN_DOWN(max_chunk, 2);
	spi_message_init_with_transfers(&m, &tr, 1);

	while (len) {
		chunk = min(len, max_chunk);
		tr.tx_buf = buf;
		tr.len = chunk;
		buf += chunk;
		len -= chunk;
		ret = spi_sync_locked(spi, &m);
		if (ret)
			return ret;
	}
	return 0;
}

static int ili9225_wr16(struct ili9225 *ctx, u8 reg, u16 v)
{
	u8 par[2] = { v >> 8, v & 0xff };
	int ret;

	spi_bus_lock(ctx->spi->controller);
	gpiod_set_value_cansleep(ctx->dc, 0);
	ret = ili9225_spi_xmit(ctx->spi, 8, &reg, 1);
	if (ret)
		goto out;
	gpiod_set_value_cansleep(ctx->dc, 1);
	ret = ili9225_spi_xmit(ctx->spi, 8, par, 2);
out:
	spi_bus_unlock(ctx->spi->controller);
	return ret;
}

static int ili9225_wr_gram(struct ili9225 *ctx, const void *data, size_t nbytes)
{
	u8 cmd = ILI9225_WRITE_DATA_TO_GRAM;
	int ret;

	spi_bus_lock(ctx->spi->controller);
	gpiod_set_value_cansleep(ctx->dc, 0);
	ret = ili9225_spi_xmit(ctx->spi, 8, &cmd, 1);
	if (ret)
		goto out;
	gpiod_set_value_cansleep(ctx->dc, 1);
	ret = ili9225_spi_xmit(ctx->spi, 16, data, nbytes);
out:
	spi_bus_unlock(ctx->spi->controller);
	return ret;
}

static void ili9225_hw_reset(struct ili9225 *ctx)
{
	if (!ctx->rst)
		return;
	gpiod_set_value_cansleep(ctx->rst, 0);
	usleep_range(20, 1000);
	gpiod_set_value_cansleep(ctx->rst, 1);
	msleep(120);
}

static int ili9225_set_window(struct ili9225 *ctx, u16 x1, u16 x2, u16 y1, u16 y2)
{
	int ret;

	ret = ili9225_wr16(ctx, ILI9225_HORIZ_WINDOW_ADDR_1, x2);
	if (ret)
		return ret;
	ret = ili9225_wr16(ctx, ILI9225_HORIZ_WINDOW_ADDR_2, x1);
	if (ret)
		return ret;
	ret = ili9225_wr16(ctx, ILI9225_VERT_WINDOW_ADDR_1, y2);
	if (ret)
		return ret;
	ret = ili9225_wr16(ctx, ILI9225_VERT_WINDOW_ADDR_2, y1);
	if (ret)
		return ret;
	ret = ili9225_wr16(ctx, ILI9225_RAM_ADDRESS_SET_1, x1);
	if (ret)
		return ret;
	return ili9225_wr16(ctx, ILI9225_RAM_ADDRESS_SET_2, y1);
}

static void ili9225_maybe_swap(void *dst, const void *src, size_t nbyte)
{
	u16 *d = dst;
	const u16 *s = src;
	size_t n = nbyte / 2;

	while (n--) {
		u16 v = *s++;

		*d++ = (v >> 8) | (v << 8);
	}
}

static int ili9225_blit_frame(struct ili9225 *ctx, const void *vaddr)
{
	u8 am_id = 0x30;
	int ret;

	switch (ctx->rotation) {
	case 90:
		am_id = 0x18;
		break;
	case 180:
		am_id = 0x00;
		break;
	case 270:
		am_id = 0x28;
		break;
	default:
		break;
	}
	ret = ili9225_wr16(ctx, ILI9225_ENTRY_MODE, 0x1000 | am_id);
	if (ret)
		return ret;
	ret = ili9225_set_window(ctx, 0, WIDTH - 1, 0, HEIGHT - 1);
	if (ret)
		return ret;

	if (ctx->swap_bytes) {
		u16 *tmp = kvmalloc(FRAME_SZ, GFP_KERNEL);

		if (!tmp)
			return -ENOMEM;
		ili9225_maybe_swap(tmp, vaddr, FRAME_SZ);
		ret = ili9225_wr_gram(ctx, tmp, FRAME_SZ);
		kvfree(tmp);
	} else {
		ret = ili9225_wr_gram(ctx, vaddr, FRAME_SZ);
	}
	return ret;
}

static int ili9225_panel_init(struct ili9225 *ctx)
{
	u8 am_id = 0x30;
	int ret;

	ili9225_hw_reset(ctx);

	switch (ctx->rotation) {
	case 90:
		am_id = 0x18;
		break;
	case 180:
		am_id = 0x00;
		break;
	case 270:
		am_id = 0x28;
		break;
	default:
		break;
	}

	ret = ili9225_wr16(ctx, ILI9225_POWER_CONTROL_1, 0x0000);
	if (ret)
		return ret;
	ili9225_wr16(ctx, ILI9225_POWER_CONTROL_2, 0x0000);
	ili9225_wr16(ctx, ILI9225_POWER_CONTROL_3, 0x0000);
	ili9225_wr16(ctx, ILI9225_POWER_CONTROL_4, 0x0000);
	ili9225_wr16(ctx, ILI9225_POWER_CONTROL_5, 0x0000);
	msleep(40);
	ili9225_wr16(ctx, ILI9225_POWER_CONTROL_2, 0x0018);
	ili9225_wr16(ctx, ILI9225_POWER_CONTROL_3, 0x6121);
	ili9225_wr16(ctx, ILI9225_POWER_CONTROL_4, 0x006f);
	ili9225_wr16(ctx, ILI9225_POWER_CONTROL_5, 0x495f);
	ili9225_wr16(ctx, ILI9225_POWER_CONTROL_1, 0x0800);
	msleep(10);
	ili9225_wr16(ctx, ILI9225_POWER_CONTROL_2, 0x103b);
	msleep(50);

	ili9225_wr16(ctx, ILI9225_DRIVER_OUTPUT_CONTROL, 0x011c);
	ili9225_wr16(ctx, ILI9225_LCD_AC_DRIVING_CONTROL, 0x0100);
	ili9225_wr16(ctx, ILI9225_ENTRY_MODE, 0x1000 | am_id);
	ili9225_wr16(ctx, ILI9225_DISPLAY_CONTROL_1, 0x0000);
	ili9225_wr16(ctx, ILI9225_BLANK_PERIOD_CONTROL_1, 0x0808);
	ili9225_wr16(ctx, ILI9225_FRAME_CYCLE_CONTROL, 0x1100);
	ili9225_wr16(ctx, ILI9225_INTERFACE_CONTROL, 0x0000);
	ili9225_wr16(ctx, ILI9225_OSCILLATION_CONTROL, 0x0d01);
	ili9225_wr16(ctx, ILI9225_VCI_RECYCLING, 0x0020);
	ili9225_wr16(ctx, ILI9225_RAM_ADDRESS_SET_1, 0x0000);
	ili9225_wr16(ctx, ILI9225_RAM_ADDRESS_SET_2, 0x0000);
	ili9225_wr16(ctx, ILI9225_GATE_SCAN_CONTROL, 0x0000);
	ili9225_wr16(ctx, ILI9225_VERTICAL_SCROLL_1, 0x00db);
	ili9225_wr16(ctx, ILI9225_VERTICAL_SCROLL_2, 0x0000);
	ili9225_wr16(ctx, ILI9225_VERTICAL_SCROLL_3, 0x0000);
	ili9225_wr16(ctx, ILI9225_PARTIAL_DRIVING_POS_1, 0x00db);
	ili9225_wr16(ctx, ILI9225_PARTIAL_DRIVING_POS_2, 0x0000);
	ili9225_wr16(ctx, ILI9225_GAMMA_CONTROL_1, 0x0000);
	ili9225_wr16(ctx, ILI9225_GAMMA_CONTROL_2, 0x0808);
	ili9225_wr16(ctx, ILI9225_GAMMA_CONTROL_3, 0x080a);
	ili9225_wr16(ctx, ILI9225_GAMMA_CONTROL_4, 0x000a);
	ili9225_wr16(ctx, ILI9225_GAMMA_CONTROL_5, 0x0a08);
	ili9225_wr16(ctx, ILI9225_GAMMA_CONTROL_6, 0x0808);
	ili9225_wr16(ctx, ILI9225_GAMMA_CONTROL_7, 0x0000);
	ili9225_wr16(ctx, ILI9225_GAMMA_CONTROL_8, 0x0a00);
	ili9225_wr16(ctx, ILI9225_GAMMA_CONTROL_9, 0x0710);
	ili9225_wr16(ctx, ILI9225_GAMMA_CONTROL_10, 0x0710);
	ili9225_wr16(ctx, ILI9225_DISPLAY_CONTROL_1, 0x0012);
	msleep(50);
	ili9225_wr16(ctx, ILI9225_DISPLAY_CONTROL_1, 0x1017);
	return 0;
}

/* --- V4L2 / vb2 --- */

static void ili9225_return_bufs(struct ili9225 *ctx, enum vb2_buffer_state state)
{
	unsigned long flags;

	while (1) {
		struct ili9225_buf *b;

		spin_lock_irqsave(&ctx->slock, flags);
		if (list_empty(&ctx->buf_list)) {
			spin_unlock_irqrestore(&ctx->slock, flags);
			break;
		}
		b = list_first_entry(&ctx->buf_list, struct ili9225_buf, list);
		list_del_init(&b->list);
		spin_unlock_irqrestore(&ctx->slock, flags);

		vb2_buffer_done(&b->vb.vb2_buf, state);
	}
}

static void ili9225_work(struct work_struct *work)
{
	struct ili9225 *ctx = container_of(work, struct ili9225, work);

	while (1) {
		struct ili9225_buf *buf = NULL;
		unsigned long flags;
		void *vaddr;
		int ret;

		spin_lock_irqsave(&ctx->slock, flags);
		if (!list_empty(&ctx->buf_list)) {
			buf = list_first_entry(&ctx->buf_list, struct ili9225_buf,
					       list);
			list_del_init(&buf->list);
		}
		spin_unlock_irqrestore(&ctx->slock, flags);

		if (!buf)
			break;

		if (!ctx->streaming) {
			vb2_buffer_done(&buf->vb.vb2_buf, VB2_BUF_STATE_ERROR);
			continue;
		}
		vaddr = vb2_plane_vaddr(&buf->vb.vb2_buf, 0);
		ret = ili9225_blit_frame(ctx, vaddr);
		vb2_buffer_done(&buf->vb.vb2_buf,
				 ret ? VB2_BUF_STATE_ERROR : VB2_BUF_STATE_DONE);
	}
}

static int ili9225_queue_setup(struct vb2_queue *vq, unsigned int *nbuffers,
			       unsigned int *nplanes, unsigned int sizes[],
			       struct device *alloc_devs[])
{
	if (*nplanes)
		return sizes[0] < FRAME_SZ ? -EINVAL : 0;
	*nplanes = 1;
	sizes[0] = FRAME_SZ;
	return 0;
}

static int ili9225_buf_prepare(struct vb2_buffer *vb)
{
	if (vb2_plane_size(vb, 0) < FRAME_SZ)
		return -EINVAL;
	vb2_set_plane_payload(vb, 0, FRAME_SZ);
	return 0;
}

static void ili9225_buf_queue(struct vb2_buffer *vb)
{
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct ili9225 *ctx = vb2_get_drv_priv(vb->vb2_queue);
	struct ili9225_buf *buf = container_of(vbuf, struct ili9225_buf, vb);
	unsigned long flags;

	spin_lock_irqsave(&ctx->slock, flags);
	list_add_tail(&buf->list, &ctx->buf_list);
	spin_unlock_irqrestore(&ctx->slock, flags);
	schedule_work(&ctx->work);
}

static int ili9225_start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct ili9225 *ctx = vb2_get_drv_priv(vq);

	ctx->streaming = true;
	return 0;
}

static void ili9225_stop_streaming(struct vb2_queue *vq)
{
	struct ili9225 *ctx = vb2_get_drv_priv(vq);

	ctx->streaming = false;
	cancel_work_sync(&ctx->work);
	ili9225_return_bufs(ctx, VB2_BUF_STATE_ERROR);
}

static const struct vb2_ops ili9225_vb2_ops = {
	.queue_setup = ili9225_queue_setup,
	.buf_prepare = ili9225_buf_prepare,
	.buf_queue = ili9225_buf_queue,
	.start_streaming = ili9225_start_streaming,
	.stop_streaming = ili9225_stop_streaming,
	.wait_prepare = vb2_ops_wait_prepare,
	.wait_finish = vb2_ops_wait_finish,
};

static int ili9225_querycap(struct file *file, void *fh,
			    struct v4l2_capability *cap)
{
	strscpy(cap->driver, "ili9225_v4l2", sizeof(cap->driver));
	strscpy(cap->card, "ILI9225 SPI", sizeof(cap->card));
	strscpy(cap->bus_info, "SPI", sizeof(cap->bus_info));
	cap->device_caps = V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_STREAMING;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;
	return 0;
}

static int ili9225_enum_fmt(struct file *file, void *fh,
			    struct v4l2_fmtdesc *f)
{
	if (f->index > 0)
		return -EINVAL;
	f->pixelformat = V4L2_PIX_FMT_RGB565;
	return 0;
}

static int ili9225_g_fmt(struct file *file, void *fh, struct v4l2_format *f)
{
	if (f->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;
	f->fmt.pix.width = WIDTH;
	f->fmt.pix.height = HEIGHT;
	f->fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
	f->fmt.pix.field = V4L2_FIELD_NONE;
	f->fmt.pix.bytesperline = WIDTH * 2;
	f->fmt.pix.sizeimage = FRAME_SZ;
	f->fmt.pix.colorspace = V4L2_COLORSPACE_DEFAULT;
	return 0;
}

static int ili9225_try_fmt(struct file *file, void *fh, struct v4l2_format *f)
{
	if (f->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;
	if (f->fmt.pix.pixelformat != V4L2_PIX_FMT_RGB565)
		f->fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
	f->fmt.pix.width = WIDTH;
	f->fmt.pix.height = HEIGHT;
	f->fmt.pix.field = V4L2_FIELD_NONE;
	f->fmt.pix.bytesperline = WIDTH * 2;
	f->fmt.pix.sizeimage = FRAME_SZ;
	f->fmt.pix.colorspace = V4L2_COLORSPACE_DEFAULT;
	return 0;
}

static int ili9225_s_fmt(struct file *file, void *fh, struct v4l2_format *f)
{
	struct ili9225 *ctx = video_drvdata(file);

	if (f->type != V4L2_BUF_TYPE_VIDEO_OUTPUT)
		return -EINVAL;
	if (f->fmt.pix.pixelformat != V4L2_PIX_FMT_RGB565)
		return -EINVAL;
	if (vb2_is_busy(&ctx->queue))
		return -EBUSY;
	return ili9225_try_fmt(file, fh, f);
}

static const struct v4l2_ioctl_ops ili9225_ioctl_ops = {
	.vidioc_querycap = ili9225_querycap,
	.vidioc_enum_fmt_vid_out = ili9225_enum_fmt,
	.vidioc_g_fmt_vid_out = ili9225_g_fmt,
	.vidioc_try_fmt_vid_out = ili9225_try_fmt,
	.vidioc_s_fmt_vid_out = ili9225_s_fmt,
	.vidioc_reqbufs = vb2_ioctl_reqbufs,
	.vidioc_create_bufs = vb2_ioctl_create_bufs,
	.vidioc_prepare_buf = vb2_ioctl_prepare_buf,
	.vidioc_querybuf = vb2_ioctl_querybuf,
	.vidioc_qbuf = vb2_ioctl_qbuf,
	.vidioc_dqbuf = vb2_ioctl_dqbuf,
	.vidioc_streamon = vb2_ioctl_streamon,
	.vidioc_streamoff = vb2_ioctl_streamoff,
};

static const struct v4l2_file_operations ili9225_fops = {
	.owner = THIS_MODULE,
	.open = v4l2_fop_open,
	.release = vb2_fop_release,
	.unlocked_ioctl = video_ioctl2,
	.mmap = vb2_fop_mmap,
	.poll = vb2_fop_poll,
};

static int ili9225_probe(struct spi_device *spi)
{
	struct ili9225 *ctx;
	struct v4l2_device *v4l2_dev;
	struct vb2_queue *q;
	struct gpio_desc *rs;
	int ret;

	ctx = devm_kzalloc(&spi->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->spi = spi;
	INIT_LIST_HEAD(&ctx->buf_list);
	spin_lock_init(&ctx->slock);
	mutex_init(&ctx->lock);
	INIT_WORK(&ctx->work, ili9225_work);

	ctx->rst = devm_gpiod_get(&spi->dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->rst))
		return dev_err_probe(&spi->dev, PTR_ERR(ctx->rst),
				     "reset GPIO\n");

	rs = devm_gpiod_get(&spi->dev, "dc", GPIOD_OUT_LOW);
	if (IS_ERR(rs)) {
		rs = devm_gpiod_get(&spi->dev, "rs", GPIOD_OUT_LOW);
		if (IS_ERR(rs))
			return dev_err_probe(&spi->dev, PTR_ERR(rs),
					     "dc/rs GPIO\n");
	}
	ctx->dc = rs;

	ctx->backlight = devm_gpiod_get_optional(&spi->dev, "backlight",
						 GPIOD_OUT_LOW);
	if (IS_ERR(ctx->backlight))
		return dev_err_probe(&spi->dev, PTR_ERR(ctx->backlight),
				     "backlight GPIO\n");

	device_property_read_u32(&spi->dev, "rotation", &ctx->rotation);
	ctx->swap_bytes = device_property_read_bool(&spi->dev, "swap-bytes");

	ret = ili9225_panel_init(ctx);
	if (ret)
		return dev_err_probe(&spi->dev, ret, "panel init\n");

	v4l2_dev = &ctx->v4l2_dev;
	ret = v4l2_device_register(&spi->dev, v4l2_dev);
	if (ret)
		return ret;

	q = &ctx->queue;
	q->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	q->io_modes = VB2_MMAP | VB2_USERPTR | VB2_DMABUF;
	q->drv_priv = ctx;
	q->buf_struct_size = sizeof(struct ili9225_buf);
	q->ops = &ili9225_vb2_ops;
	q->mem_ops = &vb2_vmalloc_memops;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	q->min_buffers_needed = 1;
	q->dev = &spi->dev;

	ret = vb2_queue_init(q);
	if (ret)
		goto unreg_v4l2;

	ctx->vdev.device_caps = V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_STREAMING;
	ctx->vdev.fops = &ili9225_fops;
	ctx->vdev.release = video_device_release_empty;
	ctx->vdev.v4l2_dev = v4l2_dev;
	ctx->vdev.queue = q;
	ctx->vdev.ioctl_ops = &ili9225_ioctl_ops;
	ctx->vdev.lock = &ctx->lock;
	strscpy(ctx->vdev.name, "ili9225-out", sizeof(ctx->vdev.name));

	video_set_drvdata(&ctx->vdev, ctx);
	ret = video_register_device(&ctx->vdev, VFL_TYPE_VIDEO, -1);
	if (ret) {
		dev_err(&spi->dev, "video_register_device failed\n");
		goto rel_queue;
	}

	spi_set_drvdata(spi, ctx);
	if (ctx->backlight)
		gpiod_set_value_cansleep(ctx->backlight, 1);
	v4l2_info(v4l2_dev, "registered %s\n", video_device_node_name(
			   &ctx->vdev));
	return 0;

rel_queue:
	vb2_queue_release(q);
unreg_v4l2:
	v4l2_device_unregister(v4l2_dev);
	return ret;
}

static void ili9225_remove(struct spi_device *spi)
{
	struct ili9225 *ctx = spi_get_drvdata(spi);

	if (!ctx)
		return;

	cancel_work_sync(&ctx->work);
	video_unregister_device(&ctx->vdev);
	vb2_queue_release(&ctx->queue);
	v4l2_device_unregister(&ctx->v4l2_dev);
	if (ctx->backlight)
		gpiod_set_value_cansleep(ctx->backlight, 0);
}

static const struct spi_device_id ili9225_v4l2_id[] = {
	{ "ili9225_v4l2", 0 },
	{}
};
MODULE_DEVICE_TABLE(spi, ili9225_v4l2_id);

static const struct of_device_id ili9225_v4l2_of_match[] = {
	{ .compatible = "ilitek,ili9225" },
	{},
};
MODULE_DEVICE_TABLE(of, ili9225_v4l2_of_match);
MODULE_ALIAS("of:N*T*Cilitek,ili9225C*");

static struct spi_driver ili9225_v4l2_driver = {
	.driver = {
		.name = "ili9225_v4l2",
		.of_match_table = ili9225_v4l2_of_match,
	},
	.id_table = ili9225_v4l2_id,
	.probe = ili9225_probe,
	.remove = ili9225_remove,
};
module_spi_driver(ili9225_v4l2_driver);

MODULE_DESCRIPTION("ILI9225 SPI panel V4L2 video output 176x220");
MODULE_AUTHOR("Local");
MODULE_LICENSE("GPL");
