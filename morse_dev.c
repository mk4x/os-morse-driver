/* Prototype module for second mandatory DM510 assignment */
#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

/* ioctl commands */
#define MORSE_SET_UNIT_DURATION _IOW('m', 1, int)
#define MORSE_GET_UNIT_DURATION _IOR('m', 2, int)
#define MORSE_SET_BUFFER_SIZE   _IOW('m', 3, int)
#define MORSE_GET_BUFFER_SIZE   _IOR('m', 4, int)

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
#include <linux/mutex.h>

#include "morse_table.h"
#include "gpio_handler.h"

/* Morse timing from morse_table.c */
extern int unit_duration;

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
#define BUFFER_DEFAULT_SIZE 1024
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

/* circular buffer */
struct circ_buf {
    char *data;
    int size;
    int read_pos; /* a kthread will read here, head */
    int write_pos; /* write() will write here, tail */
    struct mutex lock; /* atomic lock only for this buffer*/
    wait_queue_head_t  readers; /* ktrread_pos sleep here when empty */
    wait_queue_head_t  writers; /* write() sleeps here when full */
};

/* initialise the cbuffer ring */
static int cbuf_init(struct circ_buf *b, int size)
{
    b->data = kmalloc(size, GFP_KERNEL);
    if (!b->data)
        return -ENOMEM;
    b->size = size;
    b->read_pos = 0;
    b->write_pos = 0;
    mutex_init(&b->lock);
    init_waitqueue_head(&b->readers);
    init_waitqueue_head(&b->writers);
    return 0;
}

/* cleanup for cbuffer */
static void cbuf_destroy(struct circ_buf *b)
{
    kfree(b->data);
    b->data = NULL;
}

static inline int cbuf_has_data(struct circ_buf *b)
{
    /* if write_pos has caught up to read_pos, everything is read */
    return b->read_pos != b->write_pos;
}

static inline int cbuf_has_space(struct circ_buf *b)
{
    /* the case where write pos + 1 would result in being
       equal to the read_pos, meaning no more space left
    */
    return ((b->write_pos+1) % b->size) != b->read_pos;
}

static inline void cbuf_put(struct circ_buf *b, char c)
{
    b->data[b->write_pos] = c;
    b->write_pos = (b->write_pos + 1) % b->size;
}

static inline char cbuf_get(struct circ_buf *b)
{
    char c = b->data[b->read_pos];
    b->read_pos = (b->read_pos + 1) % b->size;
    return c;
}

static struct circ_buf     morse_buf;
static struct task_struct *morse_thread;
static atomic_t            morse_open_count = ATOMIC_INIT(0);

/* morse kthread livecycle, responsible for waking and reacting to when data is in circ_buf */
static int morse_kthread(void *data)
{
    char c;

    printk(KERN_INFO "Morse: kthread started.\n");

    /* built in command, if kthread is killed from outside*/
    while (!kthread_should_stop())
    {
        /* let the thread sleep until buffer has data or if called to kill itself */
        if (wait_event_interruptible(morse_buf.readers, cbuf_has_data(&morse_buf) || kthread_should_stop()))
        {
            break;
        }

        /* kill itself */
        if (kthread_should_stop())
            break;

        /* lock the data for this thread, and take 1 char out */
        mutex_lock(&morse_buf.lock);
        /* data could have not have reached morse_write before the thread woke up, check again to be sure */
        if (!cbuf_has_data(&morse_buf))
        {
            /* no data to read, unlock again */
            mutex_unlock(&morse_buf.lock);
            continue;
        }

        /* read data */
        c = cbuf_get(&morse_buf);
        mutex_unlock(&morse_buf.lock);

        /* tell writers, if they are blocked, that they can write again since a slot is freed up */
        wake_up_interruptible(&morse_buf.writers);

        /* transmit the data to the LED */
        /* note  this should not be done inside the lock since its a slow operation with msleep,
        morse_write cant happen if we are waiting */
        transmit_morse(&c, 1); // 1 char
    }
    return 0;
}


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

    /* Allocate the cbuf_ring */
    err = cbuf_init(&morse_buf, BUFFER_DEFAULT_SIZE);
    if (err) {
        printk(KERN_WARNING "Morse: buffer allocation failed\n");
        cdev_del(&morse_cdev);
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

    /* Start the kthread */
    morse_thread = kthread_run(morse_kthread, NULL, "morse_kthread");
    if (IS_ERR(morse_thread)) {
        printk(KERN_WARNING "Morse: kthread_run failed\n");
        cbuf_destroy(&morse_buf);
        cdev_del(&morse_cdev);
        unregister_chrdev_region(morse_devno, DEVICE_COUNT);
        return PTR_ERR(morse_thread);
    }


	printk(KERN_INFO "Morse: Morse loaded, device (%d, %d)!\n", MAJOR_NUMBER, MIN_MINOR_NUMBER);

	return 0;
}

/* Called when module is unloaded */
void morse_cleanup_module( void ) {
    /* kill the thread first*/
    wake_up_interruptible(&morse_buf.readers);
    kthread_stop(morse_thread);

	led_off();

    /* Unmap GPIO memory, handles !gpio_base itself*/
    gpio_hw_exit();

    /* */
    cbuf_destroy(&morse_buf);

    cdev_del(&morse_cdev);
    unregister_chrdev_region(morse_devno, DEVICE_COUNT);

	printk(KERN_INFO "Morse: Module unloaded.\n");
}


/* Called when a process tries to open the device file */
static int morse_open( struct inode *inode, struct file *filp ) {

    /* if it is already claimed */
	if (atomic_inc_return(&morse_open_count) > 1) {
        atomic_dec(&morse_open_count);
        return -EBUSY;
    }
    printk(KERN_INFO "Morse: device opened.\n");
    return 0;
}


/* Called when a process closes the device file. */
static int morse_release( struct inode *inode, struct file *filp ) {

	atomic_dec(&morse_open_count); // remove the block
    printk(KERN_INFO "Morse: device closed.\n");
    return 0;
}


/* Called when a process writes to dev file */
static ssize_t morse_write( struct file *filp,
    const char *buf,/* The buffer to get data from      */
    size_t count,   /* The max number of bytes to write */
    loff_t *f_pos )  /* The offset in the file          */
{
    size_t i;
    char c;
    char *kbuf;

    if (count == 0)
        return 0;

    kbuf = kmalloc(count + 1, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    if (copy_from_user(kbuf, buf, count)) {
        kfree(kbuf);
        return -EFAULT;
    }

    /* write to the kernel dmesg */
    kbuf[count] = '\0'; // endline
    printk(KERN_INFO "Morse: user wrote \"%s\"\n", kbuf);
    kfree(kbuf);

    for (i = 0; i < count; i++) {

        /* Read one character from userspace */
        if (copy_from_user(&c, buf + i, 1))
            return -EFAULT;

        mutex_lock(&morse_buf.lock);

        /* If buffer is full, either bail or sleep */
        if (!cbuf_has_space(&morse_buf)) {
            mutex_unlock(&morse_buf.lock);

            if (filp->f_flags & O_NONBLOCK)
                return -EAGAIN;

            /* Block until kthread frees a slot */
            wait_event_interruptible(morse_buf.writers, cbuf_has_space(&morse_buf));
            mutex_lock(&morse_buf.lock);
        }

        cbuf_put(&morse_buf, c);
        mutex_unlock(&morse_buf.lock);
        wake_up_interruptible(&morse_buf.readers); // tell readers we wrote something
    }

    return count;
}

/* called by system call icotl */
long morse_ioctl(
    struct file *filp,
    unsigned int cmd,   /* command passed from the user */
    unsigned long arg ) /* argument of the command */
{
    int val;

    printk(KERN_INFO "Morse: ioctl called with cmd %u\n", cmd);

    switch(cmd) {
        case MORSE_SET_UNIT_DURATION:
            /* Get value from userspace */
            if (copy_from_user(&val, (int __user *)arg, sizeof(val)))
                return -EFAULT;

            /* Validate range (10ms to 1000ms) */
            if (val < 10 || val > 1000)
                return -EINVAL;

            /* Set the unit duration */
            unit_duration = val;
            printk(KERN_INFO "Morse: Unit duration set to %d ms\n", unit_duration);
            break;

        case MORSE_GET_UNIT_DURATION:
            /* Return current unit duration to userspace */
            if (copy_to_user((int __user *)arg, &unit_duration, sizeof(unit_duration)))
                return -EFAULT;
            break;

        case MORSE_SET_BUFFER_SIZE:
            /* Get new buffer size from userspace */
            if (copy_from_user(&val, (int __user *)arg, sizeof(val)))
                return -EFAULT;

            /* Validate range (64 to 4096 bytes) */
            if (val < 64 || val > 4096)
                return -EINVAL;

            /* Lock the buffer during reallocation */
            mutex_lock(&morse_buf.lock);

            /* Wait until buffer is empty before resizing */
            if (cbuf_has_data(&morse_buf)) {
                mutex_unlock(&morse_buf.lock);
                printk(KERN_WARNING "Morse: Cannot resize buffer while data pending\n");
                return -EBUSY;
            }

            /* Reallocate buffer with new size */
            kfree(morse_buf.data);
            morse_buf.data = kmalloc(val, GFP_KERNEL);
            if (!morse_buf.data) {
                mutex_unlock(&morse_buf.lock);
                return -ENOMEM;
            }
            morse_buf.size = val;
            morse_buf.read_pos = 0;
            morse_buf.write_pos = 0;

            mutex_unlock(&morse_buf.lock);

            printk(KERN_INFO "Morse: Buffer size changed to %d bytes\n", val);
            break;

        case MORSE_GET_BUFFER_SIZE:
            /* Return current buffer size */
            val = morse_buf.size;
            if (copy_to_user((int __user *)arg, &val, sizeof(val)))
                return -EFAULT;
            break;

        default:
            /* Unknown command */
            return -ENOTTY;
    }

    return 0;
}

module_init( morse_init_module );
module_exit( morse_cleanup_module );

MODULE_AUTHOR( "...Mar-Joh, Jen-Wil" );
MODULE_LICENSE( "GPL" );
