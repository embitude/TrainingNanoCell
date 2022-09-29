#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/errno.h>

#include <asm/io.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#define	MAC_ADDR_REG_START 0

/* TODO 1: Update the Vendor ID & Product ID as per the device */
#define EXPT_VENDOR_ID PCI_VENDOR_ID_REALTEK
#define EXPT_PRODUCT_ID 0x8136 /* Fast ethernet card on PCs */

static struct dev_priv
{
	void __iomem *reg_base;
	dev_t dev;
	struct cdev c_dev;
	struct class *cl;
} _pvt;

static int expt_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Expt: In open\n");
    /* TODO 18: Assign _pvt to private_data field */
	return 0;
}
static int expt_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Expt: In close\n");
	return 0;
}

static ssize_t expt_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    /* TODO 19: Get the private data structure */
	struct dev_priv *dpv;
	int i, to_read;
	uint8_t addr;

	printk(KERN_INFO "Expt: In read - Buf: %p; Len: %d; Off: %Ld\nData:\n", buf, len, *off);

	if (*off >= 6)
		return 0;

	to_read = min(len, 6 - (size_t)*off);

    /* TODO 20: Read the mac address ioread8 & send it to user space */
	
	return to_read;
}

static struct file_operations fops =
{
	.open = expt_open,
	.release = expt_close,
	.read = expt_read,
};

static int char_register_dev(struct dev_priv *dpv)
{
	int ret;
	struct device *dev_ret;

	if ((ret = alloc_chrdev_region(&dpv->dev, 0, 1, "expt_pci")) < 0)
	{
		return ret;
	}
	printk(KERN_INFO "(Major, Minor): (%d, %d)\n", MAJOR(dpv->dev), MINOR(dpv->dev));

	cdev_init(&dpv->c_dev, &fops);
	if ((ret = cdev_add(&dpv->c_dev, dpv->dev, 1)) < 0)
	{
		unregister_chrdev_region(dpv->dev, 1);
		return ret;
	}

	if (IS_ERR(dpv->cl = class_create(THIS_MODULE, "pci")))
	{
		cdev_del(&dpv->c_dev);
		unregister_chrdev_region(dpv->dev, 1);
		return PTR_ERR(dpv->cl);
	}
	if (IS_ERR(dev_ret = device_create(dpv->cl, NULL, dpv->dev, NULL, "expt%d", 0)))
	{
		class_destroy(dpv->cl);
		cdev_del(&dpv->c_dev);
		unregister_chrdev_region(dpv->dev, 1);
		return PTR_ERR(dev_ret);
	}
	return 0;
}

static void char_deregister_dev(struct dev_priv *dpv)
{
	device_destroy(dpv->cl, dpv->dev);
	class_destroy(dpv->cl);
	cdev_del(&dpv->c_dev);
	unregister_chrdev_region(dpv->dev, 1);
}

static int expt_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    int retval, i;
	uint8_t addr;
    /* TODO 6: Define the private data structure */
	struct dev_priv *dpv = &_pvt; // Should be converted to kmalloc for multi-card support
	printk(KERN_INFO "PCI device registered\n");

    /* TODO 7: Enable the pci device */
	retval = pci_enable_device(dev);
	if (retval)
	{
		printk(KERN_ERR "Unable to enable this PCI device\n");
		return retval;
	}
	else
	{
		printk(KERN_INFO "PCI device enabled\n");
	}

    /* TODO 8: Request the PCI regions */
	retval = pci_request_regions(dev, "expt_pci");
	if (retval)
	{
		printk(KERN_ERR "Unable to acquire regions of this PCI device\n");
		pci_disable_device(dev);
		return retval;
	}
	else
	{
		printk(KERN_INFO "PCI device regions acquired\n");
	}

    /* 
     * TODO 9: Get the virtual address for the pci region and 
     * assign to dpv->reg_base 
     */
	if ((dpv->reg_base = ioremap(pci_resource_start(dev, 2), pci_resource_len(dev, 2))) == NULL)
	{
		printk(KERN_ERR "Unable to map registers of this PCI device\n");
		pci_release_regions(dev);
		pci_disable_device(dev);
		return -ENODEV;
	}
	printk(KERN_INFO "Register Base: %p\n", dpv->reg_base);

    /* TODO 10: Store the private data structure into the private data area */
	pci_set_drvdata(dev, dpv);

    printk(KERN_INFO "Mac address is\n"); 
    for (i = 0; i < 6; i++)
	{
        /* TODO 11: Read the mac address ioread8 & print the same */
		addr = ioread8(dpv->reg_base + MAC_ADDR_REG_START + i);
        printk(KERN_INFO "%x", addr);
	}
    printk("\n");

    /* TODO 16: Register the character driver */
	if (retval)
	{
		/* Something prevented us from registering this driver */
		printk(KERN_ERR "Unable to register the character vertical\n");
		pci_set_drvdata(dev, NULL);
		iounmap(dpv->reg_base);
		pci_release_regions(dev);
		pci_disable_device(dev);
		return retval;
	}
	else
	{
		printk(KERN_INFO "Character vertical registered\n");
	}

	return 0;
}

static void expt_remove(struct pci_dev *dev)
{
    /* TODO 12: Get the dev_priv pointer from private data area */
	struct dev_priv *dpv = pci_get_drvdata(dev);

	pci_set_drvdata(dev, NULL);

    /* TODO 17: De-register the character driver */
	printk(KERN_INFO "Character vertical deregistered\n");

    /* TODO 13: Unmap the pci region */
	iounmap(dpv->reg_base);
	printk(KERN_INFO "PCI device memory unmapped\n");

    /* TODO 14: Release the pci regions */
	pci_release_regions(dev);
	printk(KERN_INFO "PCI device regions released\n");

    /* TODO 15: Disable the PCI device */
	pci_disable_device(dev);
	printk(KERN_INFO "PCI device disabled\n");

	printk(KERN_INFO "PCI device unregistered\n");
}

/* TODO 2: Populate the pci_device_id table, use PCI_DEVICE */
/* Table of devices that work with this driver */
static struct pci_device_id expt_table[] =
{
	{
		
        PCI_DEVICE(EXPT_VENDOR_ID, EXPT_PRODUCT_ID)
	},
	{} /* Terminating entry */
};
MODULE_DEVICE_TABLE (pci, expt_table);

/* TODO 3: Populate the pci driver fields - .name, id_table, .probe, .remove */
static struct pci_driver pci_drv =
{
	.name = "expt_pci",
	.probe = expt_probe,
	.remove = expt_remove,
	.id_table = expt_table,
};

static int __init expt_pci_init(void)
{
	int result;

	/* Register this driver with the PCI subsystem */
    /* TODO 4: Register the PCI driver */
	if ((result = pci_register_driver(&pci_drv)))
	{
		printk(KERN_ERR "pci_register_driver failed. Error number %d\n", result);
	}
	printk(KERN_INFO "Expt PCI driver registered\n");
	return result;
}

static void __exit expt_pci_exit(void)
{
	/* Deregister this driver with the PCI subsystem */
    /* TODO 5: Unregister the PCI driver */
	pci_unregister_driver(&pci_drv);
	printk(KERN_INFO "Expt PCI driver unregistered\n");
}

module_init(expt_pci_init);
module_exit(expt_pci_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pradeep Tewani");
MODULE_DESCRIPTION("Experimental PCI Device Driver");
