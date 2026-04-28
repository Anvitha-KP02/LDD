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
	{ 0x15ba50a6, "jiffies" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0x54b1fac6, "__ubsan_handle_load_invalid_value" },
	{ 0xe118efe, "snd_pcm_period_elapsed" },
	{ 0x76ba6cb4, "snd_pcm_lib_malloc_pages" },
	{ 0x4c03a563, "random_kmalloc_seed" },
	{ 0x1004e946, "kmalloc_caches" },
	{ 0xbf55f104, "kmalloc_trace" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0xd3ef067a, "snd_card_new" },
	{ 0x9f14d614, "snd_pcm_new" },
	{ 0x9e2bea7f, "snd_pcm_set_ops" },
	{ 0x33b57971, "snd_pcm_lib_preallocate_pages_for_all" },
	{ 0x17cef3e0, "snd_card_register" },
	{ 0xd09aa24e, "misc_register" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xa7eedcc4, "call_usermodehelper" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x71949ac0, "snd_pcm_lib_ioctl" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x34db050b, "_raw_spin_lock_irqsave" },
	{ 0xd35cce70, "_raw_spin_unlock_irqrestore" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x122c3a7e, "_printk" },
	{ 0x377a94a9, "snd_pcm_lib_free_pages" },
	{ 0x82ee90dc, "timer_delete_sync" },
	{ 0x4943c3e, "misc_deregister" },
	{ 0x3f4f1a3a, "snd_pcm_lib_preallocate_free_for_all" },
	{ 0x5196a707, "snd_card_free" },
	{ 0x37a0cba, "kfree" },
	{ 0x73776b79, "module_layout" },
};

MODULE_INFO(depends, "snd-pcm,snd");


MODULE_INFO(srcversion, "90B7497C76EF846D161FE9D");
