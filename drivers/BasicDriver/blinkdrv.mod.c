#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

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
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xfb384d37, "kasprintf" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xf301d0c, "kmalloc_caches" },
	{ 0x35789eee, "kmem_cache_alloc_trace" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0x8b340b22, "usb_control_msg" },
	{ 0x37a0cba, "kfree" },
	{ 0x85df9b6c, "strsep" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x92997ed8, "_printk" },
	{ 0xe74c831f, "usb_control_msg_recv" },
	{ 0xa916b694, "strnlen" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0x8198d69c, "usb_find_interface" },
	{ 0x296695f, "refcount_warn_saturate" },
	{ 0xcd5646bf, "usb_get_dev" },
	{ 0xe638dc6a, "usb_register_dev" },
	{ 0xe4e71ba2, "_dev_err" },
	{ 0xd6263894, "_dev_info" },
	{ 0x5da2dea6, "usb_put_dev" },
	{ 0xa82d41ff, "usb_deregister_dev" },
	{ 0xaca7784b, "usb_register_driver" },
	{ 0x6d7d5d36, "usb_deregister" },
	{ 0x541a6db8, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v20A0p41E5d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "470084CDC070D2D49C71694");
