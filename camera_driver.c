#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-vmalloc.h>

#define DISP_WIDTH 128
#define DISP_HEIGHT 128

struct spi_display {
	struct spi_device *spi;
};

struct cam_spi_dev {
	struct platform_device *pdev;
	struct v4l2_device vdev;
	struct v4l2_subdev sd;
	struct vb2_queue vb_queue;
	struct spi_display disp;
};

static struct cam_spi_dev *global_dev;

/* Send buffer to SPI */
static int spi_display_write(struct spi_display *disp, u8 *buf, size_t len)
{
	struct spi_transfer t = {
		.tx_buf = buf,
		.len = len,
	};
	struct spi_message m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return spi_sync(disp->spi, &m);
}

/* Init display */
static int spi_display_init(struct spi_display *disp)
{
	u8 init_seq[] = {0xAE, 0xA1, 0xC8, 0xAF};
	return spi_display_write(disp, init_seq, sizeof(init_seq));
}

/* Convert RGB888 -> RGB565 and send */
static void display_frame(struct spi_display *disp, u8 *frame)
{
	u16 rgb565[DISP_WIDTH * DISP_HEIGHT];
	int i;

	for (i = 0; i < DISP_WIDTH * DISP_HEIGHT; i++) {
		u8 r = frame[3*i];
		u8 g = frame[3*i+1];
		u8 b = frame[3*i+2];
		rgb565[i] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	}

	spi_display_write(disp, (u8*)rgb565, sizeof(rgb565));
}

/* VB2 buffer done callback */
static void my_vb2_buf_queue(struct vb2_buffer *vb)
{
	u8 *frame = vb2_plane_vaddr(vb, 0);
	if (!frame)
		return;

	display_frame(&global_dev->disp, frame);

	/* Complete the VB2 buffer */
	vb2_buffer_done(vb, VB2_BUF_STATE_DONE);
}

/* VB2 operations */
static struct vb2_ops cam_vb2_ops = {
	.queue_setup = NULL,  /* minimal example */
	.buf_queue   = my_vb2_buf_queue,
	.wait_prepare = vb2_ops_wait_prepare,
	.wait_finish  = vb2_ops_wait_finish,
};

/* Initialize VB2 queue */
static int cam_vb2_queue_init(struct cam_spi_dev *dev)
{
	struct vb2_queue *q = &dev->vb_queue;
	int ret;

	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR;
	q->drv_priv = dev;
	q->buf_struct_size = sizeof(struct vb2_buffer);
	q->mem_ops = &vb2_vmalloc_memops;
	q->ops = &cam_vb2_ops;

	ret = vb2_queue_init(q);
	if (ret)
		pr_err("VB2 queue init failed: %d\n", ret);

	return ret;
}

/* Platform probe */
static int cam_spi_probe(struct platform_device *pdev)
{
	struct cam_spi_dev *dev;
	struct spi_device *spi;
	int ret;
	pr_err("Proe called..\n");

	dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->pdev = pdev;
	spi = to_spi_device(pdev->dev.parent);
	dev->disp.spi = spi;

	ret = spi_display_init(&dev->disp);
	if (ret) {
		pr_err("SPI display init failed: %d\n", ret);
		return ret;
	}

	ret = v4l2_device_register(&pdev->dev, &dev->vdev);
	if (ret) {
		pr_err("v4l2_device_register failed: %d\n", ret);
		return ret;
	}

	ret = cam_vb2_queue_init(dev);
	if (ret) {
		v4l2_device_unregister(&dev->vdev);
		return ret;
	}

	pr_info("OV7251-SPI driver probed\n");

	platform_set_drvdata(pdev, dev);
	global_dev = dev;

	return 0;
}

/* Platform remove */
static void cam_spi_remove(struct platform_device *pdev)
{
	struct cam_spi_dev *dev = platform_get_drvdata(pdev);

	vb2_queue_release(&dev->vb_queue);
	v4l2_device_unregister(&dev->vdev);

	pr_info("OV7251-SPI driver removed\n");
}

/* Device Tree match */
static const struct of_device_id cam_spi_of_match[] = {
	{ .compatible = "ovti,ov7251" },
	{},
};
MODULE_DEVICE_TABLE(of, cam_spi_of_match);

/* Platform driver */
static struct platform_driver cam_spi_driver = {
	.probe = cam_spi_probe,
	.remove = cam_spi_remove,
	.driver = {
		.name = "ov7251_spi_display",
		.of_match_table = cam_spi_of_match,
	},
};

module_platform_driver(cam_spi_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Likitha");
MODULE_DESCRIPTION("OV7251 live capture -> SPI TFT");
