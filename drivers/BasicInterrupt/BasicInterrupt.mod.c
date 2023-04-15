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
	{ 0x92997ed8, "_printk" },
	{ 0x2de8eb5f, "usb_submit_urb" },
	{ 0x8198d69c, "usb_find_interface" },
	{ 0x296695f, "refcount_warn_saturate" },
	{ 0xf301d0c, "kmalloc_caches" },
	{ 0x35789eee, "kmem_cache_alloc_trace" },
	{ 0xcd5646bf, "usb_get_dev" },
	{ 0x3c913dfb, "usb_alloc_urb" },
	{ 0xe638dc6a, "usb_register_dev" },
	{ 0xe4e71ba2, "_dev_err" },
	{ 0xceadc1c3, "usb_free_urb" },
	{ 0xd6263894, "_dev_info" },
	{ 0x5da2dea6, "usb_put_dev" },
	{ 0x37a0cba, "kfree" },
	{ 0x813c6f7b, "usb_kill_urb" },
	{ 0xa82d41ff, "usb_deregister_dev" },
	{ 0xaca7784b, "usb_register_driver" },
	{ 0x6d7d5d36, "usb_deregister" },
	{ 0x541a6db8, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v20A0p41E5d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "795095A9F93549BC3F39159");
