
#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>


// This directory entry will point to `/sys/kernel/debug/example2`.
static struct dentry *dir = 0;

// File `/sys/kernel/debug/example2/sum` points to this variable.
static u32 sum = 0;

// This is called when `/sys/kernel/debug/example2/add` is written to.
//TODO 5: Define the handler which adds the value to the sume
static int add_write_op(void *data, u64 value)
{
    sum += value;
	return 0;
}

// TODO 4: define the file operations associated with
// `/sys/kernel/debug/example2s/add`.
// We could define the file operations by hand, but `<linux/fs.h>` provides
// a macro for this purpose DEFINE_SIMPLE_ATTRIBUTE
//
DEFINE_SIMPLE_ATTRIBUTE(add_fops, NULL, add_write_op, "%llu\n");

// The macro has form
//     DEFINE_SIMPLE_ATTRIBUTE(fops_name, read_op, write_op, printf_fmt)
// and it defines `add_ops` to be
//     struct file_operations add_ops = {
//         .read = NULL,
//         .write = add_write_op,
//         .open = add_fops_open, // Also defined by macro
//         // ...
//     };
//
// The read function is NULL because we will later set the permissions of
// `/sys/kernel/debug/example2/add` to be write-only. Hence, the
// `add_fops.read` will never be derefenced.

int init_module(void)
{
	
    // TODO 1: Create directory `/sys/kernel/debug/example2`.
	//API: debugfs_create_dir
    dir = debugfs_create_dir("example2", 0);
    if (!dir) {
        // Abort module load.
        printk(KERN_ALERT "debugfs_example2: failed to create /sys/kernel/debug/example2\n");
        return -1;
    }

    //TODO 2: Create file `/sys/kernel/debug/example2/add` with write access
    //
    // The function signature is
    //     struct dentry* debugfs_create_file(
    //         const char *name,
    //         mode_t mode,
    //         struct dentry *parent,
    //         void *data,
    //         const struct file_operations *fops);
    //
    // Since no function in `add_fops` accesses the file`s data, we may
    // set `data = NULL`.
    //
    // See also linux-source/fs/debugfs/inode.c:debugfs_create_file
    //
    debugfs_create_file(
            "add",
            0222,
            dir,
            NULL,
            &add_fops);

    // Create file `/sys/kernel/debug/example2/sum`.
    debugfs_create_u32("sum", 0444, dir, &sum);

    return 0;
}

// This is called when the module is removed.
void cleanup_module(void)
{
    // We must manually remove the debugfs entries we created. They are not
    // automatically removed upon module removal.
	//TODO 3: Remove the debugfs entry
	//API: debugfs_remove_recursive
    debugfs_remove_recursive(dir);
}

MODULE_LICENSE("GPL");
