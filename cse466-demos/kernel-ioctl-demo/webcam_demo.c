#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("frqmod");
MODULE_DESCRIPTION("Minimal webcam demo device");
MODULE_VERSION("1.0");

struct webcam_mode {
    int width;
    int height;
};

static struct webcam_mode mode = {
    .width  = 640,
    .height = 480,
};

#define WEBCAM_IOCTL_SET_MODE 1337
#define WEBCAM_IOCTL_GET_MODE 1338

static struct proc_dir_entry *proc_entry;

static int device_open(struct inode *inode, struct file *file)
{
    pr_info("[webcam_demo] device_open\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    pr_info("[webcam_demo] device_release\n");
    return 0;
}

static ssize_t device_read(struct file *file, char __user *buffer,
                           size_t length, loff_t *offset)
{
    char kbuf[64];
    int len;

    if (*offset)
        return 0;  // EOF on second read

    len = snprintf(kbuf, sizeof(kbuf),
                   "dummy image %dx%d\n", mode.width, mode.height);
    if (len > length)
        len = length;

    copy_to_user(buffer, kbuf, len);
    *offset += len;
    return len;
}

static long device_ioctl(struct file *file, unsigned int cmd,
                         unsigned long arg)
{
    switch (cmd) {
    case WEBCAM_IOCTL_SET_MODE:
        copy_from_user(&mode, (void __user *)arg, sizeof(mode));
        break;
    case WEBCAM_IOCTL_GET_MODE:
        copy_to_user((void __user *)arg, &mode, sizeof(mode));
        break;
    default:
        return -ENOTTY;
    }
    return 0;
}

static const struct file_operations fops = {
    .owner          = THIS_MODULE,
    .open           = device_open,
    .release        = device_release,
    .read           = device_read,
    .unlocked_ioctl = device_ioctl,
};

static int __init webcam_demo_init(void)
{
    proc_entry = proc_create("webcam_demo", 0666, NULL, &fops);
    return 0;
}

static void __exit webcam_demo_exit(void)
{
    if (proc_entry)
        proc_remove(proc_entry);
}

module_init(webcam_demo_init);
module_exit(webcam_demo_exit);