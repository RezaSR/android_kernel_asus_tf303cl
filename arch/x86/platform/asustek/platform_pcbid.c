/*
 * arch/arm/mach-msm/asustek/asustek-pcbid.c
 *
 * Copyright (C) 2012 ASUSTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/gpio.h>
#include <linux/string.h>
#include <linux/mfd/intel_mid_pmic.h>
//#include <mach/board_asustek.h>
//#include <asm/mach-types.h>

#define PCBID_VALUE_INVALID 0x4E2F4100 /* N/A */

// baytrail +

//add for detect platform ID ++
static int PROJECT_ID;
static int HARDWARE_ID;
static int PCB_ID;
static int TP_ID;
//add for detect platform ID --

static int project_id;
module_param(project_id, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(PROJ_VERSION,
                 "PROJ_ID judgement");

static int hardware_id;
module_param(hardware_id, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(HW_VERSION,
                 "HW_ID judgement");

static int pcb_id;
module_param(pcb_id, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(PCB_VERSION,
                 "PCB_ID judgement");

int Read_PROJ_ID(void)
{
        pr_info("PROJECT_ID = 0x%x \n",PROJECT_ID);
        project_id = PROJECT_ID;
        return PROJECT_ID;
}
EXPORT_SYMBOL(Read_PROJ_ID);

int Read_HW_ID(void)
{
        pr_info("HARDWARE_ID = 0x%x \n",HARDWARE_ID);
        hardware_id = HARDWARE_ID;
        return HARDWARE_ID;
}
EXPORT_SYMBOL(Read_HW_ID);

int Read_PCB_ID(void)
{
        pr_info("PCB_ID = 0x%x \n",PCB_ID);
        pcb_id = PCB_ID;
        return PCB_ID;
}
EXPORT_SYMBOL(Read_PCB_ID);

int Read_TP_ID(void)
{
        pr_info("TP_ID = 0x%x \n",TP_ID);
        return TP_ID;
}
EXPORT_SYMBOL(Read_TP_ID);

int get_pcb_pin(unsigned int pin)
{
    int ret;
    if(gpio_request(pin, "PCB_ID"))
    {
        pr_info("Fail to get pcb id pin %d \n", pin);
        return 0;
    }
	
    gpio_direction_input(pin);
    //int ret; 
    ret = gpio_get_value_cansleep(pin);
	printk("SOC GPIO %d := %x\n", pin, ret);
    gpio_free(pin);
    return ret;
   
}

int get_pcb_pin1(unsigned int pin)
{
    //return get_pcb_pin(pin);
    int iov = intel_mid_pmic_readb(pin);
    printk("PMIC GPIO %x := %x\n", pin, iov);
    return (iov & 0x01);
}

static void handle_pcb_id()
{
    int tmp = 0;

    //hardware id
    HARDWARE_ID = 0;
           tmp = get_pcb_pin1(0x35);
           HARDWARE_ID |= (tmp<<2);
           PCB_ID |= (tmp);
           tmp = get_pcb_pin1(0x38);
           HARDWARE_ID |= (tmp<<1);
		   PCB_ID |= (tmp << 1);
           tmp = get_pcb_pin1(0x39);
           HARDWARE_ID |= tmp;
		   PCB_ID |= (tmp << 2);

    //MEMORY
    int memory;
        memory = 0;
        tmp = get_pcb_pin1(0x47);
        memory |= (tmp << 1);
	PCB_ID |= (tmp << 3);
        tmp = get_pcb_pin1(0x48);
        memory |= tmp;
	PCB_ID |= (tmp << 4);

    //PROJECT_ID
    PROJECT_ID = 0;
        tmp = get_pcb_pin1(0x45);
        PROJECT_ID |= (tmp << 2);
        PCB_ID |= (tmp << 5);
        tmp = get_pcb_pin1(0x49);
        PROJECT_ID |= (tmp << 1);
        PCB_ID |= (tmp << 6);
        tmp = get_pcb_pin1(0x4A);
        PROJECT_ID |= tmp;
        PCB_ID |= (tmp << 7);

    //TOUCH
    TP_ID = 0;
        tmp = get_pcb_pin(155);
        TP_ID |= ( tmp << 1);
        PCB_ID |= (tmp << 8);
        tmp = get_pcb_pin(156);
        TP_ID |= tmp;
        PCB_ID |= (tmp << 9);
    //Panel
    int panel;
        panel = 0;
        panel = get_pcb_pin(95);
        PCB_ID |= (panel << 13);
    //5M CAM
    int cam_5M;
        cam_5M = 0;
        cam_5M = get_pcb_pin(55);
        PCB_ID |= (cam_5M << 14);
	//1.2M CAM
    int cam_12M;
        cam_12M = 0;
        cam_12M = get_pcb_pin(6);
        PCB_ID |= (cam_12M << 15);

    Read_HW_ID();
    Read_PCB_ID();
    Read_PROJ_ID();
    printk("Hardware ID = %x, Project ID = %x, TP ID = %d, PCB_ID = %x\n", HARDWARE_ID, PROJECT_ID, TP_ID, PCB_ID);

}

// baytrail-

static int __init pcbid_driver_probe(struct platform_device *pdev)
{        
    printk("pcbid_driver_probe+\n");

    // baytrail+
    handle_pcb_id();
    // baytrail-

    printk("pcbid_driver_probe-\n");

    return 0;
}

static int __exit pcbid_driver_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver asustek_pcbid_driver __refdata = {
	.probe = pcbid_driver_probe,
	.remove = pcbid_driver_remove,
	.driver = {
		.name = "asustek_pcbid",
		.owner = THIS_MODULE,
	},
};

static int __init asustek_pcbid_init(void)
{	printk("asustek_pcbid_init+ do\n");
	return platform_driver_register(&asustek_pcbid_driver);
}

late_initcall(asustek_pcbid_init);
//module_init(asustek_pcbid_init);

MODULE_DESCRIPTION("ASUSTek PCBID driver");
MODULE_AUTHOR("Craig Chen <craig_chen@asus.com>");
MODULE_LICENSE("GPL");
