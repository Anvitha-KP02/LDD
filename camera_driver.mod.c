#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xfa474811, "__platform_driver_register" },
	{ 0x72a30d06, "vb2_queue_release" },
	{ 0x62d86bf7, "v4l2_device_unregister" },
	{ 0x92997ed8, "_printk" },
	{ 0xdcb764ad, "memset" },
	{ 0x20b64901, "spi_sync" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x36a78de3, "devm_kmalloc" },
	{ 0xeb1df604, "v4l2_device_register" },
	{ 0x91747f5f, "vb2_vmalloc_memops" },
	{ 0xa1ce0773, "vb2_queue_init" },
	{ 0x4097ccd3, "vb2_plane_vaddr" },
	{ 0x171dbbb7, "vb2_buffer_done" },
	{ 0x61fd46a9, "platform_driver_unregister" },
	{ 0x67edc1f2, "vb2_ops_wait_prepare" },
	{ 0xd06fe8d7, "vb2_ops_wait_finish" },
	{ 0x474e54d2, "module_layout" },
};

MODULE_INFO(depends, "videobuf2-v4l2,videodev,videobuf2-vmalloc,videobuf2-common");

MODULE_ALIAS("of:N*T*Covti,ov7251");
MODULE_ALIAS("of:N*T*Covti,ov7251C*");

MODULE_INFO(srcversion, "B81D7E7233BC6178A9074B9");
