// SPDX-License-Identifier: GPL-2.0
/*
 * entropy_mem.c
 *
 * A misc char device: /dev/entropy_mem
 * - write(): stores data and resets entropy timeline
 * - read(): returns data that physically decays over time
 *
 * v0.1.0a0 improvements:
 * - mutex for safe concurrent access
 * - stronger error handling (copy_to/from_user, bounds, zero-length, etc.)
 * - cumulative, bounded decay (no unbounded loops after long sleep)
 *
 * Permissions note:
 *   The recommended way is a udev rule (preferred) rather than chmod after insmod.
 *   Example rule (e.g. /etc/udev/rules.d/99-entropy_mem.rules):
 *       KERNEL=="entropy_mem", MODE="0660", GROUP="entropy"
 *   Then add users to group 'entropy'.
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
/*
 * Number of bytes already "decayed" (cumulative). Bounded to data_len.
 * We compute target based on elapsed_sec, and apply only (target - decayed_bytes).
 */
static size_t decayed_bytes;

static inline char random_printable_ascii(void)
{
	/* Printable ASCII range: 33..126 (94 chars) */
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

	/* target corruption bytes = elapsed seconds, capped to length */
	target = min_t(size_t, (size_t)elapsed_sec, data_len);

	if (decayed_bytes >= target)
		return;

	delta = target - decayed_bytes;

	/*
	 * Apply delta random corruptions in-place.
	 * This makes the decay "physical" (storage mutates over time).
	 */
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

	/* Allow empty write to clear state */
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

	/*
	 * Security: clear entire storage first so older secrets don't remain
	 * when writing shorter content later.
	 */
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

	/*
	 * Reset file offset to 0 for sanity; user-space can still manage its own offset,
	 * but we keep a consistent internal model.
	 */
	if (ppos)
		*ppos = 0;

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

	/* Mutate storage based on elapsed time (cumulative decay). */
	apply_decay_locked();

	/* Standard read semantics: obey *ppos and count */
	if (*ppos < 0) {
		mutex_unlock(&entropy_lock);
		return -EINVAL;
	}
	if ((size_t)*ppos >= data_len) {
		mutex_unlock(&entropy_lock);
		return 0; /* EOF */
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
	.llseek = no_llseek,
};

static struct miscdevice entropy_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "entropy_mem",       /* -> /dev/entropy_mem */
	.fops = &entropy_fops,
	/*
	 * NOTE: Some kernels support miscdevice.mode to set node permissions directly.
	 * We intentionally avoid relying on that field for portability, and recommend udev rules instead.
	 */
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

