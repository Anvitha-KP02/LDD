#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x6fcadc04, "_dev_info" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x441047d0, "_snd_pcm_stream_lock_irqsave" },
	{ 0x209bd8ee, "snd_pcm_stream_unlock_irqrestore" },
	{ 0x102fe6de, "hrtimer_cancel" },
	{ 0xc0b7c197, "hrtimer_start_range_ns" },
	{ 0x377a94a9, "snd_pcm_lib_free_pages" },
	{ 0x71949ac0, "snd_pcm_lib_ioctl" },
	{ 0x2cf56265, "__dynamic_pr_debug" },
	{ 0x37a0cba, "kfree" },
	{ 0xf4f31022, "platform_driver_unregister" },
	{ 0xbe11e457, "platform_device_unregister" },
	{ 0xd7ec758a, "platform_device_register_full" },
	{ 0x9a7b4bd7, "__platform_driver_register" },
	{ 0x4c03a563, "random_kmalloc_seed" },
	{ 0x1004e946, "kmalloc_caches" },
	{ 0xbf55f104, "kmalloc_trace" },
	{ 0xea82d349, "hrtimer_init" },
	{ 0xe118efe, "snd_pcm_period_elapsed" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0x135bb7ec, "hrtimer_forward" },
	{ 0x76ba6cb4, "snd_pcm_lib_malloc_pages" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x122c3a7e, "_printk" },
	{ 0x5196a707, "snd_card_free" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xd3ef067a, "snd_card_new" },
	{ 0xa916b694, "strnlen" },
	{ 0xdd64e639, "strscpy" },
	{ 0x656e4a6e, "snprintf" },
	{ 0x9f14d614, "snd_pcm_new" },
	{ 0x9e2bea7f, "snd_pcm_set_ops" },
	{ 0x33b57971, "snd_pcm_lib_preallocate_pages_for_all" },
	{ 0x17cef3e0, "snd_card_register" },
	{ 0x73776b79, "module_layout" },
};

MODULE_INFO(depends, "snd-pcm,snd");


MODULE_INFO(srcversion, "48CAF09770F7520473D8F6A");
