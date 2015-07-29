/* Copyright (c) 2013, ASUSTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
//#include "../gpio-names.h"

//#include <mach/board_asustek.h>

#define GPIO0P2		202
#define GPIO0P5		205
#define GPIO0P6		206
#define GPIO1P4		212
#define GPIO1P5		213
#define GPIO1P2		210
#define GPIO1P6		214
#define GPIO1P7		215
#define GPIO_S5_25	155
#define GPIO_S5_26	156
#define GPIO_S0_95	95
#define GPIO_S0_55	55
#define GPIO_S0_6	6

struct asustek_pcbid_platform_data {
        const char *UUID;
};

//#ifdef CONFIG_ASUSTEK_PCBID
static char serialno[32] = {0,};
int __init asustek_androidboot_serialno(char *s)
{
	int n;

	if (*s == '=')
		s++;
	n = snprintf(serialno, sizeof(serialno), "%s", s);
	serialno[n] = '\0';

	return 1;
}
__setup("androidboot.serialno", asustek_androidboot_serialno);

struct asustek_pcbid_platform_data asustek_pcbid_pdata = {
	.UUID = serialno,
};

static struct resource resources_asustek_pcbid[] = {
	{
		.start	= GPIO0P2,
		.end	= GPIO0P2,
		.name	= "PCB_ID0",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO0P5,
		.end	= GPIO0P5,
		.name	= "PCB_ID1",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO0P6,
		.end	= GPIO0P6,
		.name	= "PCB_ID2",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO1P4,
		.end	= GPIO1P4,
		.name	= "PCB_ID3",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO1P5,
		.end	= GPIO1P5,
		.name	= "PCB_ID4",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO1P2,
		.end	= GPIO1P2,
		.name	= "PCB_ID5",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO1P6,
		.end	= GPIO1P6,
		.name	= "PCB_ID6",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO1P7,
		.end	= GPIO1P7,
		.name	= "PCB_ID7",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_S5_25,
		.end	= GPIO_S5_25,
		.name	= "PCB_ID8",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_S5_26,
		.end	= GPIO_S5_26,
		.name	= "PCB_ID9",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_S0_95,
		.end	= GPIO_S0_95,
		.name	= "PCB_ID13",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_S0_55,
		.end	= GPIO_S0_55,
		.name	= "PCB_ID14",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_S0_6,
		.end	= GPIO_S0_6,
		.name	= "PCB_ID15",
		.flags	= IORESOURCE_IO,
	},
};

static struct platform_device asustek_pcbid_device = {
	.name		= "asustek_pcbid",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resources_asustek_pcbid),
	.resource = resources_asustek_pcbid,
	.dev = {
		.platform_data = &asustek_pcbid_pdata,
	}
};

void __init asustek_add_pcbid_devices(void)
{
	printk("asustek_add_pcbid_devices+\n");
	platform_device_register(&asustek_pcbid_device);
	printk("asustek_add_pcbid_devices-\n");
}

arch_initcall(asustek_add_pcbid_devices);
//#endif
