/**
 * i2c msg monitor
 *
 * counter for i2c bus communication , record
 * (1)record amount of i2c device pass communication
 * (2)dump fail i2c device timestamp and WARN_ON
 * (3)dump pass i2c devices record every sec
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/time.h>

#define MAX_BUS_NUM 7
#define MAX_DEV_NUM 10 //per bus
#define MAX_DEV_ERR_CODE_NUM 100
static bool flag_test_enable = false;

int i2c_devices[7][10][2] = {0};  //(7 is bus , 10 is devices ( reserve ) , 2 ( 1st addr , 2nd count )
//7 buses , 100 devices with error codes , 3(1st addr , 2nd err code , 3rd count )
int i2c_devices_err[7][100][3] = {0};
static struct timeval t_record_start_time;

struct delayed_work asus_i2c_monitor_work;
static struct workqueue_struct *asus_i2c_monitor_work_queue;
static void asus_i2c_monitor_work_function(struct work_struct * work){
    int bus=0,dev=0;
    struct timeval t_current_time;
    int diff_time = 0;
    do_gettimeofday(&t_current_time);
    diff_time = t_current_time.tv_sec-t_record_start_time.tv_sec;
    printk("ASUS I2C MONITOR : ------------ record duration : %d (sec)-----------\n",diff_time);
    for(bus=0;bus<MAX_BUS_NUM;bus++){
        for(dev=0;dev<MAX_DEV_NUM;dev++){
            if(i2c_devices[bus][dev][0]>0){
                printk("ASUS I2C MONITOR : [%d][%d] bus= %d addr= %2x count= %d\n",
                        bus,dev,(bus+1),i2c_devices[bus][dev][0],i2c_devices[bus][dev][1]);
            }
        }
    }

    for(bus=0;bus<MAX_BUS_NUM;bus++){
        for(dev=0;dev<MAX_DEV_ERR_CODE_NUM;dev++){
            if(i2c_devices_err[bus][dev][0]>0){
                printk("ASUS I2C MONITOR : ------------ error! -----------\n");
                printk("ASUS I2C MONITOR : [%d][%d] bus= %d addr= %2x err = %d count= %d\n",
                        bus,dev,(bus+1),i2c_devices_err[bus][dev][0],i2c_devices_err[bus][dev][1],i2c_devices_err[bus][dev][2]);
            }
        }
    }
    queue_delayed_work(asus_i2c_monitor_work_queue, &asus_i2c_monitor_work, msecs_to_jiffies(5000));
}
void record_i2c_msg_from_i2c_core(int bus,struct i2c_msg *msgs,int msgs_num,int err)
{
    //Note: err > 0 is PASS , err = 0 is timeout(FAIL) , err < 0 is FAIL
    int addr = 0;
    int i=0;
    if(flag_test_enable==false){
        return;
    }

    if(msgs_num > 0){
        addr = msgs[0].addr;
    }
    //bus num check
    if(bus>MAX_BUS_NUM){
        printk("ASUS I2C MONITOR : [err][over max bus num] bus=%d addr=%x err=%d\n",bus,addr,err);
        return;
    }
    //addr check
    if(addr==0){
        printk("ASUS I2C MONITOR : [err][addr is 0 ] bus=%d addr=%x err=%d\n",bus,addr,err);
        return;
    }
    //check err
    if(err>0){
        //count
        for(i=0;i<MAX_DEV_NUM;i++){
            if(i2c_devices[(bus-1)][i][0]==0){
                i2c_devices[(bus-1)][i][0] = addr;
                i2c_devices[(bus-1)][i][1] = 1;
                break;
            }else if(i2c_devices[(bus-1)][i][0]==addr){
                i2c_devices[(bus-1)][i][1]++;
                break;
            }else if(i==(MAX_DEV_NUM-1)){
                printk("ASUS I2C MONITOR : [err][MAX_DEV_NUM not enough] bus=%d addr=%x err=%d\n",bus,addr,err);
            }
        }

    }else{
        //print err msg and err code and WARN_ON
        for(i=0;i<MAX_DEV_ERR_CODE_NUM;i++){
            if( (i2c_devices_err[(bus-1)][i][0]==0) && (i2c_devices_err[(bus-1)][i][1]==0) ){
                i2c_devices_err[(bus-1)][i][0] = addr;
                i2c_devices_err[(bus-1)][i][1] = err;
                i2c_devices_err[(bus-1)][i][2] = 1;
                break;
            }else if( (i2c_devices_err[(bus-1)][i][0]==addr) && (i2c_devices_err[(bus-1)][i][1]==err) ){
                i2c_devices_err[(bus-1)][i][2]++;
                break;
            }else if(i==(MAX_DEV_ERR_CODE_NUM-1)){
                printk("ASUS I2C MONITOR : [err][MAX_DEV_NUM not enough] bus=%d addr=%x err=%d\n",bus,addr,err);
            }
        }
        printk("ASUS I2C MONITOR : [err][err code <=0 ] bus=%d addr=%x err=%d\n",bus,addr,err);
        WARN_ON(1);
    }
}
EXPORT_SYMBOL(record_i2c_msg_from_i2c_core);


static ssize_t asus_i2c_monitor_enable_test(struct device *dev, struct device_attribute *attr,
        const char *buf , size_t count){
    long enable;
    printk("ASUS I2C MONITOR : asus_i2c_monitor_enable_test()\n");
    if (strict_strtol(buf, count, &enable))
        return -EINVAL;
    if ((enable != 1) && (enable != 0))
        return -EINVAL;

    if(enable == 1){
        flag_test_enable = true;
        //timestamp start time temp here
        do_gettimeofday(&t_record_start_time);
        queue_delayed_work(asus_i2c_monitor_work_queue, &asus_i2c_monitor_work,  msecs_to_jiffies(1000)); // after 30 sec start work
        printk("ASUS I2C MONITOR : enable test \n");
    }else{
        flag_test_enable = false;
        cancel_delayed_work_sync(&asus_i2c_monitor_work);
        printk("ASUS I2C MONITOR : disable test \n");
    }
    return strnlen(buf, count);
}

static DEVICE_ATTR(enable_test, 0664, NULL, asus_i2c_monitor_enable_test);

/* Attribute Descriptor */
static struct attribute *asus_i2c_monitor_attrs[] = {
    &dev_attr_enable_test.attr,
    NULL
};

/* Attribute group */
static struct attribute_group asus_i2c_monitor_attr_group = {
    .attrs = asus_i2c_monitor_attrs,
};

static int __init asus_i2c_msg_monitor_probe(struct platform_device *pdev)
{
    int ret = 0;
    printk("ASUS I2C MONITOR : asus_i2c_msg_monitor_probe()\n");
    //add work queue for dump i2c_devices
    asus_i2c_monitor_work_queue = create_singlethread_workqueue("asus_i2c_monitor_wq");
    if(!asus_i2c_monitor_work_queue){
        printk("ASUS I2C MONITOR : [err] unable to create asus_i2c_monitor_wq workqueue\n");
        goto destroy_wq;
    }
    INIT_DELAYED_WORK(&asus_i2c_monitor_work, asus_i2c_monitor_work_function);

    //sysfs create
    ret = sysfs_create_group(&pdev->dev.kobj, &asus_i2c_monitor_attr_group);
    if (ret) {
        printk("ASUS I2C MONITOR : Unable to create sysfs, error: %d\n",ret);
        goto fail_sysfs;
    }

    return 0;
fail_sysfs:
    destroy_workqueue(asus_i2c_monitor_work_queue);
destroy_wq:
    return 0;
}

static int __exit asus_i2c_msg_monitor_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &asus_i2c_monitor_attr_group);
    destroy_workqueue(asus_i2c_monitor_work_queue);
    return 0;
}

static struct platform_driver asus_i2c_msg_monitor __refdata = {
    .probe = asus_i2c_msg_monitor_probe,
    .remove = __exit_p(asus_i2c_msg_monitor_remove),
    .driver = {
        .name = "asus_i2c_msg_monitor",
        .owner = THIS_MODULE,
    },
};

static int __init asus_i2c_msg_monitor_init(void)
{
    extern int entry_mode;
    if(entry_mode == 4)
        return -1;

    printk("asus_i2c_msg_monitor module init\n");
    return platform_driver_register(&asus_i2c_msg_monitor);
}

static void __exit asus_i2c_msg_monitor_exit(void)
{
    platform_driver_unregister(&asus_i2c_msg_monitor);
}

module_init(asus_i2c_msg_monitor_init);
module_exit(asus_i2c_msg_monitor_exit);

MODULE_DESCRIPTION("ASUS I2C MSG MONITOR DRIVER");
MODULE_LICENSE("GPL");
