
#include <linux/kernel.h>
#include <linux/platform_device.h>

static struct platform_device asus_i2c_monitor_device = {
    .name      = "asus_i2c_msg_monitor",
    .id        = -1,
};

static int __init asus_i2c_monitor_init(void)
{
    extern int entry_mode;
    if(entry_mode == 4)
        return -1;

    printk("asus_i2c_monitor_device init \n");
    printk("platform init ret = %d \n",platform_device_register(&asus_i2c_monitor_device));
    return 0;
}
module_init(asus_i2c_monitor_init);
