#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/cred.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pwn.college");
MODULE_DESCRIPTION("pwn.college challenge");
MODULE_VERSION("1.0");

static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t *offset);

static unsigned char *shellcode;
struct proc_dir_entry *proc_entry = NULL;

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .write = device_write,
};

int init_module(void)
{
    shellcode = __vmalloc(0x1000, GFP_KERNEL, PAGE_KERNEL_EXEC);

    proc_entry = proc_create("shellcode", 0666, NULL, &fops);

    return 0;
}

void cleanup_module(void)
{
    if (proc_entry)
        proc_remove(proc_entry);
}

static int device_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
    unsigned long remaining;
    ssize_t result = length;

    remaining = copy_from_user(shellcode, buffer, min((size_t)4096, length));
    ((void (*)(void))shellcode)();
    result = length - remaining;

    return result;
}
