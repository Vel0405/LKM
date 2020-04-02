#include <linux/module.h>  // Needed by all modules
#include <linux/kernel.h>  // Needed for KERN_INFO
#include <linux/fs.h>      // Needed by filp
#include <asm/uaccess.h>   // Needed by segment descriptors
#include<linux/delay.h>
#include<linux/kthread.h>

static struct task_struct *ts;

int check_battery(void)
{
    // Create variables
    struct file *f;
    char buf[128];
    mm_segment_t fs;
    int i;
    // Init the buffer with 0
    for(i=0;i<128;i++)
        buf[i] = 0;
    // To see in /var/log/messages that the module is operating
    printk(KERN_INFO "My module is loaded\n");
    // I am using Fedora and for the test I have chosen following file
    // Obviously it is much smaller than the 128 bytes, but hell with it =)
    f = filp_open("/sys/class/power_supply/BAT1/capacity", O_RDONLY, 0);
    if(f == NULL)
        printk(KERN_ALERT "filp_open error!!.\n");
    else{
        // Get current segment descriptor
        fs = get_fs();
        // Set segment descriptor associated to kernel space
        set_fs(KERNEL_DS);
        // Read the file
        f->f_op->read(f, buf, 128, &f->f_pos);
        // Restore segment descriptor
        set_fs(fs);
        // See what we read from file
	printk(KERN_INFO " buf:%s\n",buf[0]);
    }

    filp_close(f,NULL);
    return 0;
}

int batt_thread(void *data)
{
	while(!kthread_should_stop())
	{
		msleep(1000);
		check_battery();
	}
	return 0;
}

static int __init helloentry(void)
{
        ts=kthread_run(batt_thread,NULL,"battcheck");
	return 0;
}


static void __exit helloexit(void)
{
    printk(KERN_INFO "My module is unloaded\n");
}

module_init(helloentry);
module_exit(helloexit);
