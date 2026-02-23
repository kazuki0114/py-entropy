// SPDX-License-Identifier: GPL-2.0
/*
 * entropy_mem.c
 *
 * A misc char device: /dev/entropy_mem
 * - write(): stores data and resets entropy timeline
 * - read(): returns data that physically decays over time
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/jiffies.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/errno.h>
#include <linux/minmax.h>
#include <linux/types.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Entropy Architect");
MODULE_DESCRIPTION("A memory device that decays over time");
MODULE_VERSION("0.1.0a0");

#define MAX_SIZE 1024

static DEFINE_MUTEX(entropy_lock);

static char storage[MAX_SIZE];
static size_t data_len;
static unsigned long write_time_jiffies;
static size_t decayed_bytes;

static inline char random_printable_ascii(void)
{
    u32 r = get_random_u32();
    return (char)((r % 94) + 33);
}

static void apply_decay_locked(void)
{
    unsigned long now = jiffies;
    unsigned long elapsed_sec;
    size_t target, delta;
    size_t i;

    if (data_len == 0)
        return;

    elapsed_sec = (now - write_time_jiffies) / HZ;
    target = min_t(size_t, (size_t)elapsed_sec, data_len);

    if (decayed_bytes >= target)
        return;

    delta = target - decayed_bytes;

    for (i = 0; i < delta; i++) {
        u32 pos = get_random_u32() % data_len;
        storage[pos] = random_printable_ascii();
    }

    decayed_bytes = target;
}

static ssize_t entropy_write(struct file *file,
                 const char __user *user_buf,
                 size_t count,
                 loff_t *ppos)
{
    size_t copy_len;
    int ret;

    if (count == 0) {
        if (mutex_lock_interruptible(&entropy_lock))
            return -ERESTARTSYS;

        memset(storage, 0, sizeof(storage));
        data_len = 0;
        write_time_jiffies = jiffies;
        decayed_bytes = 0;

        mutex_unlock(&entropy_lock);
        return 0;
    }

    copy_len = min_t(size_t, count, (size_t)MAX_SIZE - 1);

    if (mutex_lock_interruptible(&entropy_lock))
        return -ERESTARTSYS;

    memset(storage, 0, sizeof(storage));

    ret = copy_from_user(storage, user_buf, copy_len);
    if (ret != 0) {
        mutex_unlock(&entropy_lock);
        return -EFAULT;
    }

    storage[copy_len] = '\0';
    data_len = copy_len;
    write_time_jiffies = jiffies;
    decayed_bytes = 0;

    mutex_unlock(&entropy_lock);
    return (ssize_t)copy_len;
}

static ssize_t entropy_read(struct file *file,
                char __user *user_buf,
                size_t count,
                loff_t *ppos)
{
    size_t to_copy;

    if (!user_buf || !ppos)
        return -EINVAL;

    if (count == 0)
        return 0;

    if (mutex_lock_interruptible(&entropy_lock))
        return -ERESTARTSYS;

    if (data_len == 0) {
        mutex_unlock(&entropy_lock);
        return 0;
    }

    apply_decay_locked();

    if (*ppos < 0) {
        mutex_unlock(&entropy_lock);
        return -EINVAL;
    }
    if ((size_t)*ppos >= data_len) {
        mutex_unlock(&entropy_lock);
        return 0;
    }

    to_copy = min_t(size_t, count, data_len - (size_t)*ppos);

    if (copy_to_user(user_buf, storage + *ppos, to_copy)) {
        mutex_unlock(&entropy_lock);
        return -EFAULT;
    }

    *ppos += to_copy;

    mutex_unlock(&entropy_lock);
    return (ssize_t)to_copy;
}

static const struct file_operations entropy_fops = {
    .owner = THIS_MODULE,
    .write = entropy_write,
    .read  = entropy_read,
    // 【修正点】 Pythonからの lseek を正しく処理できるように変更
    .llseek = default_llseek,
};

static struct miscdevice entropy_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "entropy_mem",
    .fops = &entropy_fops,
};

static int __init entropy_init(void)
{
    int rc;

    rc = misc_register(&entropy_device);
    if (rc) {
        pr_err("entropy_mem: failed to register misc device: %d\n", rc);
        return rc;
    }

    pr_info("entropy_mem: loaded (/dev/entropy_mem)\n");
    return 0;
}

static void __exit entropy_exit(void)
{
    misc_deregister(&entropy_device);
    pr_info("entropy_mem: unloaded\n");
}

module_init(entropy_init);
module_exit(entropy_exit);