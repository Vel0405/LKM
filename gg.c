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
    	struct file *f,*g,*h;
    	char buf[128];
	int bval=10,i,b1val=0;
    	mm_segment_t fs;
	ssize_t rk=0,rk1=0;
	size_t len = sizeof(bval);
	size_t len1=sizeof(b1val);
    // Init the buffer with 0
    	for(i=0;i<128;i++)
	{
		buf[i] = 0;
	}
    // To see in /var/log/messages that the module is operating
    	printk(KERN_INFO "My module is loaded\n");

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
		int buf1;
		if(kstrtoint(buf, 10, &buf1) != 0)
        	{
			return -1;
		}
		printk(KERN_ALERT "battery% is %d",buf1);
		char bright[20], bluet[10]; /* should be large enough */
		if(buf1<=98)
		{
			g=filp_open("/sys/class/backlight/nvidia_0/brightness",O_RDWR,0);
    			len = scnprintf(bright, sizeof(bright), "%d\n", bval);
    			rk = kernel_write(g,bright,len,&g->f_pos);
			h=filp_open("/sys/class/bluetooth/hci0/rfkill0/state",O_RDWR,0);
			len1 = scnprintf(bluet, sizeof(bluet), "%d\n", b1val);
			rk1 = kernel_write(h,bluet,len1,&h->f_pos);
	 		printk(KERN_ALERT "battery is low");
		}
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
