/*
 * platform_mt9m114.c: mt9m114 platform data initilization file
 *
 * (C) Copyright 2008 Intel Corporation
 * Author:
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/atomisp_platform.h>
#include <asm/intel-mid.h>
#include <asm/intel_scu_ipcutil.h>
#include <media/v4l2-subdev.h>
#include <linux/regulator/consumer.h>
#include <linux/sfi.h>
#include <linux/mfd/intel_mid_pmic.h>
#include <linux/vlv2_plat_clock.h>
#include "platform_camera.h"
#include "platform_mt9m114.h"
#include "platform_mt9e013.h"

/* workround - pin defined for byt */
#define CAMERA_1_RESET 127
#define CAMERA_1_PWDN 124
#define CAMERA_2P8_EN 153
#ifdef CONFIG_VLV2_PLAT_CLK
#define OSC_CAM1_CLK 0x1
#define CLK_19P2MHz 0x1
#endif
#ifdef CONFIG_CRYSTAL_COVE
#define VPROG_2P8V 0x66
#define VPROG_1P8V 0x5D
#define VPROG_ENABLE 0x3
#define VPROG_DISABLE 0x2
#endif

#define VPROG1_VAL 2800000
static int camera_reset;
static int camera_power_down;
static int camera_vprog1_on;
static int camera_2p8_gpio;


static struct regulator *vprog1_reg;
#if 0
static int is_ctp(void)
{
	return INTEL_MID_BOARD(1, PHONE, CLVTP)
		|| INTEL_MID_BOARD(1, TABLET, CLVT);
}
#endif
/*
 * MFLD PR2 secondary camera sensor - MT9M114 platform data
 */
static int mt9m114_gpio_ctrl(struct v4l2_subdev *sd, int flag)
{
#if 0
	int ret;

	if (camera_reset < 0) {
		ret = camera_sensor_gpio(-1, GP_CAMERA_1_RESET,
					 GPIOF_DIR_OUT, 1);
		if (ret < 0)
			return ret;
		camera_reset = ret;
	}

	if (flag) {
		if (is_ctp()) {
			gpio_set_value(camera_reset, 0);
			msleep(60);
		}
		gpio_set_value(camera_reset, 1);
	} else
		gpio_set_value(camera_reset, 0);
#endif

//ASUS_BSP+++
        int ret;
        int pin;
        /*
        * FIXME: WA using hardcoded GPIO value here.
        * The GPIO value would be provided by ACPI table, which is
        * not implemented currently.
        */
        pin = CAMERA_1_RESET;
        if (camera_reset < 0) {
                ret = gpio_request(pin, "camera_1_reset");
                if (ret) {
                        pr_err("%s: failed to request gpio(pin %d)\n", __func__, pin);
                        return ret;
                }
        }
        camera_reset = pin;
        ret = gpio_direction_output(pin, 1);
        if (ret) {
                pr_err("%s: failed to set gpio(pin %d) direction\n", __func__, pin);
                gpio_free(pin);
                return ret;
        }

if (flag) {
#ifdef CONFIG_BOARD_CTP
        gpio_set_value(camera_reset, 0);
        msleep(60);
#endif
        gpio_set_value(camera_reset, 1);
} else {
        gpio_set_value(camera_reset, 0);
        gpio_free(camera_reset);
        camera_reset = -1;
}
        return 0;
//ASUS_BSP---
}

static int mt9m114_flisclk_ctrl(struct v4l2_subdev *sd, int flag)
{
//ASUS_BSP+++
	static const unsigned int clock_khz = 19200;
#if 0
	return intel_scu_ipc_osc_clk(OSC_CLK_CAM1, flag ? clock_khz : 0);
#endif
        int ret = 0;
#ifdef CONFIG_INTEL_SCU_IPC_UTIL
        if (intel_mid_identify_cpu() != INTEL_MID_CPU_CHIP_VALLEYVIEW2)
                return intel_scu_ipc_osc_clk(OSC_CLK_CAM1, flag ? clock_khz : 0);
#endif
#ifdef CONFIG_VLV2_PLAT_CLK
        if (flag) {
                ret = vlv2_plat_set_clock_freq(OSC_CAM1_CLK, CLK_19P2MHz);
                if (ret)
                        return ret;
        }
        ret = vlv2_plat_configure_clock(OSC_CAM1_CLK, flag);
#endif
        return ret;
//ASUS_BSP---
}

#ifndef CONFIG_BOARD_CTP
static int mt9e013_reset_value;
#endif

static int mt9m114_power_ctrl(struct v4l2_subdev *sd, int flag)
{
//ASUS_BSP+++
#if 0
	int reg_err;

	if (!is_ctp()) {
		int ret;

		/* Note here, there maybe a workaround to avoid I2C SDA issue */
		if (camera_power_down < 0) {
			ret = camera_sensor_gpio(-1, GP_CAMERA_1_POWER_DOWN,
						GPIOF_DIR_OUT, 1);
#ifndef CONFIG_BOARD_REDRIDGE
			if (ret < 0)
				return ret;
#endif
			camera_power_down = ret;
		}

		if (camera_reset < 0) {
			ret = camera_sensor_gpio(-1, GP_CAMERA_1_RESET,
						 GPIOF_DIR_OUT, 1);
			if (ret < 0)
				return ret;
			camera_reset = ret;
		}
	}

	if (flag) {
		if (!is_ctp()) {
			if (!mt9e013_reset_value) {
				if (mt9e013_reset)
					mt9e013_reset(sd);
				mt9e013_reset_value = 1;
			}
#ifdef CONFIG_BOARD_REDRIDGE
			gpio_direction_output(camera_reset, 0);
#endif
			gpio_set_value(camera_reset, 0);
		}
		if (!camera_vprog1_on) {
			camera_vprog1_on = 1;
			if (is_ctp()) {
				reg_err = regulator_enable(vprog1_reg);
				if (reg_err) {
					printk(KERN_ALERT "Failed to enable"
						" regulator vprog1\n");
					return reg_err;
				}
			} else {
				intel_scu_ipc_msic_vprog1(1);
			}
		}
		if (!is_ctp()) {
#ifdef CONFIG_BOARD_REDRIDGE
			if (camera_power_down >= 0)
				gpio_set_value(camera_power_down, 1);
#else
			gpio_set_value(camera_power_down, 1);
#endif
		}
	} else {
		if (camera_vprog1_on) {
			camera_vprog1_on = 0;
			if (is_ctp()) {
				reg_err = regulator_disable(vprog1_reg);
				if (reg_err) {
					printk(KERN_ALERT "Failed to disable"
						" regulator vprog1\n");
					return reg_err;
				}
			} else {
				intel_scu_ipc_msic_vprog1(0);
			}
		}
		if (!is_ctp()) {
#ifdef CONFIG_BOARD_REDRIDGE
			if (camera_power_down >= 0)
				gpio_set_value(camera_power_down, 0);
#else
			gpio_set_value(camera_power_down, 0);
#endif

			mt9e013_reset_value = 0;
		}
	}
#endif
        int ret = 0, pin = 0;

        if (flag) {
                if (!camera_vprog1_on) {
#ifdef CONFIG_INTEL_SCU_IPC_UTIL
                        if (intel_mid_identify_cpu() != INTEL_MID_CPU_CHIP_VALLEYVIEW2)
                                ret = intel_scu_ipc_msic_vprog1(1);
#endif
#ifdef CONFIG_CRYSTAL_COVE
                        /*
                        * This should call VRF APIs.
                        *
                        * VRF not implemented for BTY, so call this
                        * as WAs
                        */

                        //turn on 1.8V
                        printk("%s turn on 1.8V\n", __func__ );
                        ret = camera_set_pmic_power(CAMERA_1P8V, true);
                        if (ret){
                                pr_err("%s fail to turn on 1.8V\n", __func__);
                                return ret;
                        }

                        //turn on 2.8V
                        printk("%s turn on 2.8V\n", __func__ );
                        ret = camera_set_pmic_power(CAMERA_2P8V, true);
                        if (ret){
                                pr_err("%s fail to turn on 2.8V\n", __func__);
                                return ret;
                        }

                        //set CAM_2P8_GPIO high
                        printk("%s set high CAM_2P8_GPIO\n", __func__ );
                        pin = CAMERA_2P8_EN;
                        if (camera_2p8_gpio < 0) {
                                ret = gpio_request(pin, "camera_2p8_gpio");
                                if (ret) {
                                        pr_err("%s: failed to request gpio(pin %d)\n", __func__, pin);
                                        return ret;
                                }
                         }
                        camera_2p8_gpio = pin;
                        ret = gpio_direction_output(pin, 1);
                        if (ret) {
                                pr_err("%s: failed to set gpio(pin %d) direction\n", __func__, pin);
                                gpio_free(pin);
                                return ret;
                        }
                        gpio_set_value(camera_2p8_gpio, 1);
                        msleep(1);

                        //set EXTCLK 19.2MHz
                        printk("%s set EXTCLK 19.2MHz\n", __func__ );
                        ret = mt9m114_flisclk_ctrl(sd, 1);
                        if (ret) {
                                pr_err("%s fail to set EXTCLK 19.2MHz\n", __func__);
                                return ret;
                        }

                        //set RESET_BAR
                        printk("%s set RESET_BAR\n", __func__ );
                        ret = mt9m114_gpio_ctrl(sd, 1);
                        if (ret) {
                                pr_err("%s fail to set RESET_BAR\n", __func__);
                                return ret;
                        }
#endif
                        if (!ret)
                                camera_vprog1_on = 1;
                        return ret;
                }
        } else {
                if (camera_vprog1_on) {
#ifdef CONFIG_INTEL_SCU_IPC_UTIL
                        if (intel_mid_identify_cpu() != INTEL_MID_CPU_CHIP_VALLEYVIEW2)
                                ret = intel_scu_ipc_msic_vprog1(0);
#endif
#ifdef CONFIG_CRYSTAL_COVE

                        //release EXTCLK 19.2MHz
                        printk("%s release EXTCLK 19.2MHz\n", __func__ );
                        ret = mt9m114_flisclk_ctrl(sd, 2);
                        if (ret) {
                                pr_err("%s fail to release EXTCLK 19.2MHz\n", __func__);
                                return ret;
                        }

                        //release RESET_BAR
                        printk("%s release RESET_BAR\n", __func__ );
                        ret = mt9m114_gpio_ctrl(sd, 0);
                        if (ret) {
                                pr_err("%s fail to set RESET_BAR\n", __func__);
                                return ret;
                        }

                        //set CAM_2P8_GPIO low
                        printk("%s set low CAM_2P8_GPIO\n", __func__ );
                        pin = CAMERA_2P8_EN;
                        if (camera_2p8_gpio < 0) {
                                ret = gpio_request(pin, "camera_2p8_gpio");
                                if (ret) {
                                        pr_err("%s: failed to request gpio(pin %d)\n", __func__, pin);
                                        return ret;
                                }
                         }
                        camera_2p8_gpio = pin;
                        ret = gpio_direction_output(pin, 1);
                        if (ret) {
                                pr_err("%s: failed to set gpio(pin %d) direction\n", __func__, pin);
                                gpio_free(pin);
                                return ret;
                        }
                        gpio_set_value(camera_2p8_gpio, 0);
                        gpio_free(camera_2p8_gpio);
                        camera_2p8_gpio = -1;

                        //turn off 2.8V
                        printk("%s turn off 2.8V\n", __func__ );
                        ret = camera_set_pmic_power(CAMERA_2P8V, false);
                        if (ret){
                                pr_err("%s fail to turn off 2.8V\n", __func__);
                                return ret;
                        }

                        //turn off 1.8V
                        printk("%s turn off 1.8V\n", __func__ );
                        ret = camera_set_pmic_power(CAMERA_1P8V, false);
                        if (ret){
                                pr_err("%s fail to turn off 1.8V\n", __func__);
                                return ret;
                        }
#endif
                        if (!ret)
                                camera_vprog1_on = 0;
                        return ret;
                }
        }
        return 0;
//ASUS_BSP---
}

static int mt9m114_csi_configure(struct v4l2_subdev *sd, int flag)
{
	return camera_sensor_csi(sd, ATOMISP_CAMERA_PORT_SECONDARY, 1,
                ATOMISP_INPUT_FORMAT_RAW_10, atomisp_bayer_order_grbg, flag);
}

static int mt9m114_platform_init(struct i2c_client *client)
{
	int ret;

	vprog1_reg = regulator_get(&client->dev, "vprog1");
	if (IS_ERR(vprog1_reg)) {
		dev_err(&client->dev, "regulator_get failed\n");
		return PTR_ERR(vprog1_reg);
	}
	ret = regulator_set_voltage(vprog1_reg, VPROG1_VAL, VPROG1_VAL);
	if (ret) {
		dev_err(&client->dev, "regulator voltage set failed\n");
		regulator_put(vprog1_reg);
	}
	return ret;
}

static int mt9m114_platform_deinit(void)
{
	regulator_put(vprog1_reg);

	return 0;
}

static struct camera_sensor_platform_data mt9m114_sensor_platform_data = {
	.gpio_ctrl	= mt9m114_gpio_ctrl,
	.flisclk_ctrl	= mt9m114_flisclk_ctrl,
	.power_ctrl	= mt9m114_power_ctrl,
	.csi_cfg	= mt9m114_csi_configure,
//ASUS_BSP+++
#ifdef CONFIG_BOARD_CTP
        .platform_init = mt9m114_platform_init,
        .platform_deinit = mt9m114_platform_deinit,
#endif
//ASUS_BSP---
};

void *mt9m114_platform_data(void *info)
{
	camera_reset = -1;
	camera_power_down = -1;
        camera_2p8_gpio = -1;
//ASUS_BSP+++
#if 0
	if (is_ctp()) {
		mt9m114_sensor_platform_data.platform_init =
			mt9m114_platform_init;
		mt9m114_sensor_platform_data.platform_deinit =
			mt9m114_platform_deinit;
	}
#endif
//ASUS_BSP---
	return &mt9m114_sensor_platform_data;
}

