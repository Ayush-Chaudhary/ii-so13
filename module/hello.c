/* 2014
 * Maciej Szeptuch
 * IIUWr
 * ----------
 *  Simple linux kernel device driver. "Hello World!"
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

static dev_t        device;
static struct cdev  char_device;
static struct class *cls;

static int device_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int device_close(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    const char  *msg        = "Hello World!\n";
    ssize_t     bytes_read  = 0;
    int         pos         = *offset % 13;

    while(length --)
    {
        ++ bytes_read;
        put_user(msg[pos ++], buffer ++);
        if(pos == 13)
            pos = 0;
    }

    return bytes_read;
}

static struct file_operations file_ops =
{
    .owner      = THIS_MODULE,
    .open       = device_open,
    .release    = device_close,
    .read       = device_read,
};

int init_module(void)
{
    printk(KERN_INFO "Loading Hello World! device\n");
    if(alloc_chrdev_region(&device, 0, 1, "hello") < 0)
        return -1;

    if((cls = class_create(THIS_MODULE, "chardrv")) == NULL)
    {
        unregister_chrdev_region(device, 1);
        return -1;
    }

    if(device_create(cls, NULL, device, NULL, "hello") == NULL)
    {
        class_destroy(cls);
        unregister_chrdev_region(device, 1);
        return -1;
    }

    cdev_init(&char_device, &file_ops);
    if(cdev_add(&char_device, device, 1) == -1)
    {
        device_destroy(cls, device);
        class_destroy(cls);
        unregister_chrdev_region(device, 1);
        return -1;
    }

    return 0;
}

void cleanup_module(void)
{
    cdev_del(&char_device);
    device_destroy(cls, device);
    class_destroy(cls);
    unregister_chrdev_region(device, 1);
    printk(KERN_INFO "Hello World! driver unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Maciej Szeptuch <neverous@neverous.info>");
MODULE_DESCRIPTION("Hello World! device driver");
