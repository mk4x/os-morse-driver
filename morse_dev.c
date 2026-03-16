/* Prototype module for second mandatory DM510 assignment */
#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>	
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/wait.h>
/* #include <asm/uaccess.h> */
#include <linux/uaccess.h>
#include <linux/semaphore.h>
/* #include <asm/system.h> */
#include <asm/switch_to.h>
#include <linux/cdev.h>
#include "morse_table.h"
#include "gpio_handler.h"

/* Prototypes - this would normally go in a .h file */
int morse_init_module( void );
void morse_cleanup_module( void );
static int morse_open( struct inode*, struct file* );
static int morse_release( struct inode*, struct file* );
static ssize_t morse_write( struct file*, const char*, size_t, loff_t* );
long morse_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#define DEVICE_NAME "morse_dev" /* Dev name as it appears in /proc/devices */
#define MAJOR_NUMBER 255
#define MIN_MINOR_NUMBER 0
#define MAX_MINOR_NUMBER 1

#define DEVICE_COUNT 1
/* end of what really should have been in a .h file */


/* file operations struct */
static struct file_operations morse_fops = {
	.owner   = THIS_MODULE,
	.write   = morse_write,
	.open    = morse_open,
	.release = morse_release,
    .unlocked_ioctl = morse_ioctl
};

/* cdev and devno */
static struct cdev  morse_cdev;
static dev_t        morse_devno;



/* called when module is loaded */
int morse_init_module( void ) {
    int err;

    /* Claim device number for driver*/
    morse_devno = MKDEV(MAJOR_NUMBER, MIN_MINOR_NUMBER);
    err = register_chrdev_region(morse_devno, DEVICE_COUNT, DEVICE_NAME);
    if (err < 0) {
        printk(KERN_WARNING "Morse: cannot register device number: %d\n", err);
        return err;
    }

    /* Connects file_operations to device number */
    cdev_init(&morse_cdev, &morse_fops);
    morse_cdev.owner = THIS_MODULE;
    err = cdev_add(&morse_cdev, morse_devno, DEVICE_COUNT);
    if (err) {
        printk(KERN_WARNING "Morse: cdev_add failed: %d\n", err);
        unregister_chrdev_region(morse_devno, DEVICE_COUNT);
        return err;
    }

    /* Map GPIO registers into virtual space */
    gpio_hw_init(); /* Will fill gpio_base* pointer */
    if (!gpio_base) {
        printk(KERN_WARNING "Morse: ioremap failed\n");
        cdev_del(&morse_cdev);
        unregister_chrdev_region(morse_devno, DEVICE_COUNT);
        return -ENOMEM;
    }

    /* Setup LED */
    gpio_set_mode(GPIO_LED_PIN, 1); // 1 = output
    led_off();


	printk(KERN_INFO "Morse: Morse loaded, device (%d, %d)!\n", MAJOR_NUMBER, MIN_MINOR_NUMBER);

	return 0;
}

/* Called when module is unloaded */
void morse_cleanup_module( void ) {
	led_off();
 
    /* Unmap GPIO memory, handles !gpio_base itself*/
    gpio_hw_exit(void); 
 
    cdev_del(&morse_cdev);
    unregister_chrdev_region(morse_devno, DEVICE_COUNT);
 
	printk(KERN_INFO "Morse: Module unloaded.\n");
}


/* Called when a process tries to open the device file */
static int morse_open( struct inode *inode, struct file *filp ) {
	
	/* device claiming code belongs here */
    printk(KERN_INFO "Morse: device opened.\n");
    /* 
    TODO: Later, implement synchronzation here so two devices 
    cannot write to the same device.
    */
	return 0;
}


/* Called when a process closes the device file. */
static int morse_release( struct inode *inode, struct file *filp ) {

	/* device release code belongs here */
	printk(KERN_INFO "Morse: device closed.\n");
    /* 
    TODO: Later, implement synchronzation... Free the thread here.
    */
	return 0;
}


/* Called when a process writes to dev file */
static ssize_t morse_write( struct file *filp,
    const char *buf,/* The buffer to get data from      */
    size_t count,   /* The max number of bytes to write */
    loff_t *f_pos )  /* The offset in the file           */
{

	/* write code belongs here */	

	return 0; //return number of bytes written
}

/* called by system call icotl */ 
long morse_ioctl( 
    struct file *filp, 
    unsigned int cmd,   /* command passed from the user */
    unsigned long arg ) /* argument of the command */
{
	/* ioctl code belongs here */
	printk(KERN_INFO "Morse: ioctl called.\n");

	return 0; //has to be changed
}

module_init( morse_init_module );
module_exit( morse_cleanup_module );

MODULE_AUTHOR( "...Mar-Joh, Jen-Wil" );
MODULE_LICENSE( "GPL" );
