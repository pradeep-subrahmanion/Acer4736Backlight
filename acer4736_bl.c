/*
 * Backlight driver for Acer Aspire 4736
 *
 * Copyright (C) Pradeep Subrahmanion <subrahmanion.pradeep@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * 
 * This driver uses LBB PCI configuration register to change the 
 * backlight brightness.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/dmi.h>
#include <linux/io.h>
#include <linux/pci.h>

static u8 max_brightness = 0xFF;
static u8 lbb_offset =  0xF4;
static unsigned int device_id = 0x2a42;
static unsigned int vendor_id = 0x8086;

struct backlight_device *acer_backlight_device;
struct pci_dev *pdev;

static int acer_dmi_match(const struct dmi_system_id *id)
{
	printk(KERN_INFO "acer4736_bl: %s detected\n", id->ident);
	return 1;
}

static const struct dmi_system_id __initdata acer_device_table[] = {
{
		.callback	= acer_dmi_match,
		.ident		= "Aspire 4736",
		.matches	= {
			DMI_MATCH(DMI_SYS_VENDOR, "Acer"),
			DMI_MATCH(DMI_PRODUCT_NAME, "Aspire 4736"),
		},
	},
	{}
};
static int read_brightness(struct backlight_device *bd)
{
	u8 result;
	pci_read_config_byte(pdev, lbb_offset, &result);
	return result;
}

static int update_brightness(struct backlight_device *bd)
{
	u8 intensity = bd->props.brightness;
	if (intensity > max_brightness) {
		printk(KERN_INFO "Acer4736_bl: Invalid parameter. Maximum value is %d"
		, max_brightness);
		return -1;
	}
	pci_write_config_byte(pdev, lbb_offset, intensity);
	return 0;
}
static const struct  backlight_ops acer_backlight_ops = {
	.get_brightness = read_brightness,
	.update_status  = update_brightness,
};

static int __init acer4736_bl_init(void)
{
	struct backlight_properties props;
	if (!dmi_check_system(acer_device_table))
		return -ENODEV;

	pdev = pci_get_device(vendor_id, device_id, NULL);

	if (!pdev)
		return -ENODEV;

	printk(KERN_INFO "Loading Acer 4736 backlight driver\n");
	memset(&props, 0, sizeof(struct backlight_properties));
	props.type = BACKLIGHT_RAW;
	props.max_brightness = max_brightness;

	acer_backlight_device = backlight_device_register("acer_backlight",
		NULL, NULL, &acer_backlight_ops, &props);
	acer_backlight_device->props.max_brightness = max_brightness;
	acer_backlight_device->props.brightness  =
		read_brightness(acer_backlight_device);
	backlight_update_status(acer_backlight_device);
	
	return 0;
}

static void __exit acer4736_bl_exit(void)
{
	pci_dev_put(pdev);
	backlight_device_unregister(acer_backlight_device);
}

module_init(acer4736_bl_init);
module_exit(acer4736_bl_exit);

MODULE_AUTHOR("Pradeep Subrahmanion <subrahmanion.pradeep@gmail.com>");
MODULE_DESCRIPTION("Acer Aspire 4736 Backlight Driver");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(dmi, acer_device_table);
