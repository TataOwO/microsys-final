#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>

#define MAJOR_NUM 60
#define MODULE_NAME "demo"

static int iCount = 0;
static char userChar[100];

//  LED status
static char led_status[4] = {'0', '0', '0', '0'}; //  LED1 ~ LED4

// Open function
static int drv_open(struct inode *inode, struct file *filp) {
    	printk("Enter Open function\n"); // [cite: 303]
    	return 0;
}

// Release function
static int drv_release(struct inode *inode, struct file *filp) {
    	printk("Enter Release function\n"); // [cite: 304]
    	return 0;
}

static void file_write(char *path, char *str) {
    struct file *fp;
    loff_t pos = 0;
    mm_segment_t old_fs;

    old_fs = get_fs();
    set_fs(get_ds());

    fp = filp_open(path, O_WRONLY, 0);
    if (IS_ERR(fp)) {
        printk("File open error: %s\n", path);
    } else {
        vfs_write(fp, str, strlen(str), &pos);
        filp_close(fp, NULL);
    }

    set_fs(old_fs);
}

// Read function：return LED status to User
static ssize_t drv_read(struct file *filp, char *buf, size_t count, loff_t *ppos) {
    printk("Enter Read function\n"); 
    
    // [Fix] let led_status copy to user's buf
    // return recorded 4 LED status for real time
    int ret;
    if (count < 4) return -EINVAL; // Buffer 2 small
    
    ret = copy_to_user(buf, led_status, 4); 
    if (ret != 0) {
        return -EFAULT;
    }
    
    return 4; // return loaded 4 bytes
}

// Write function：receive User operation then control LED
static ssize_t drv_write(struct file *filp, const char *buf, size_t count, loff_t *ppos) {
    	printk("device write\n");
	printk("%d\n", iCount);
	printk("W_buf_size: %d\n", (int)count);
	if(count > 99){ count = 99; }	//

	copy_from_user(userChar, buf, count);
	userChar[count] = '\0';
	printk("userChar(char): %s\n", userChar);

	char *gpio_path_led1 = "/sys/class/gpio/gpio396/value";		//pin 7
	char *gpio_path_led2 = "/sys/class/gpio/gpio466/value";		//pin 11
	char *gpio_path_led3 = "/sys/class/gpio/gpio297/value";		//pin 32
	char *gpio_path_led4 = "/sys/class/gpio/gpio254/value";		//pin 22
    
    	if (strncmp(userChar, "LED1on", 6) == 0) {
        	file_write(gpio_path_led1, "1");
        	led_status[0] = '1';
        	printk("LED1 turned ON\n");
    	} else if (strncmp(userChar, "LED1off", 7) == 0) {
        	file_write(gpio_path_led1, "0");
        	led_status[0] = '0';
        	printk("LED1 turned OFF\n");
    	} else if (strncmp(userChar, "LED2on", 6) == 0) {
        	file_write(gpio_path_led2, "1");
        	led_status[1] = '1';
        	printk("LED2 turned ON\n");
    	} else if (strncmp(userChar, "LED2off", 7) == 0) {
        	file_write(gpio_path_led2, "0");
        	led_status[1] = '0';
        	printk("LED2 turned OFF\n");
    	} else if (strncmp(userChar, "LED3on", 6) == 0) {
        	file_write(gpio_path_led3, "1");
        	led_status[2] = '1';
        	printk("LED3 turned ON\n");
    	} else if (strncmp(userChar, "LED3off", 7) == 0) {
        	file_write(gpio_path_led3, "0");
        	led_status[2] = '0';
        	printk("LED3 turned OFF\n");
    	} else if (strncmp(userChar, "LED4on", 6) == 0) {
        	file_write(gpio_path_led4, "1");
        	led_status[3] = '1';
        	printk("LED4 turned ON\n");
    	} else if (strncmp(userChar, "LED4off", 7) == 0) {
        	file_write(gpio_path_led4, "0");
        	led_status[3] = '0';
        	printk("LED4 turned OFF\n");
    	}

	iCount++;
    	return count;
}

// I/O Control function
static long drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    	printk("Enter I/O Control function\n"); // [cite: 307]
    	return 0;
}

// define file_operations [cite: 1013]
struct file_operations drv_fops = {
    	.read = drv_read,
    	.write = drv_write,
    	.unlocked_ioctl = drv_ioctl,
    	.open = drv_open,
    	.release = drv_release,
};

// Init function [cite: 1028]
static int demo_init(void) {
    	if (register_chrdev(MAJOR_NUM, MODULE_NAME, &drv_fops) < 0) {
    		printk("%s: can't get major %d\n", MODULE_NAME, MAJOR_NUM);
    		return -EBUSY;
    	}
    	printk("<1>%s: started\n", MODULE_NAME);
    	return 0;
}

// Exit function [cite: 1040]
static void demo_exit(void) {
    unregister_chrdev(MAJOR_NUM, MODULE_NAME);
    printk("<1>%s: removed\n", MODULE_NAME);
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_LICENSE("GPL");
