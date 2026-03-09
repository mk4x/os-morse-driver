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

/* called when module is loaded */
int morse_init_module( void ) {

	/* initialization code belongs here */

	printk(KERN_INFO "Morse: Hello from your device!\n");

	return 0;
}

/* Called when module is unloaded */
void morse_cleanup_module( void ) {

	/* clean up code belongs here */

	printk(KERN_INFO "Morse: Module unloaded.\n");
}


/* Called when a process tries to open the device file */
static int morse_open( struct inode *inode, struct file *filp ) {
	
	/* device claiming code belongs here */

	return 0;
;
}


/* Called when a process closes the device file. */
static int morse_release( struct inode *inode, struct file *filp ) {

	/* device release code belongs here */
		
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

MODULE_AUTHOR( "...Your names here. Do not delete the three dots in the beginning." );
MODULE_LICENSE( "GPL" );
