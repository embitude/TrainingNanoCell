#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/errno.h>

#include <asm/io.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

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

static int expt_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    int retval, i;
	uint8_t addr;
    struct dev_priv *dpv;
    /* TODO 6: Assign the driver private data struct to dpv */
	printk(KERN_INFO "PCI device registered\n");

    /* TODO 7: Enable the pci device */
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
	if ((dpv->reg_base) == NULL)
	{
		printk(KERN_ERR "Unable to map registers of this PCI device\n");
		pci_release_regions(dev);
		pci_disable_device(dev);
		return -ENODEV;
	}
	printk(KERN_INFO "Register Base: %p\n", dpv->reg_base);

    /* TODO 10: Store the private data structure into the private data area */

    printk(KERN_INFO "Mac address is\n"); 
    /* TODO 11: Read the mac address ioread8 & print the same */
   
	return 0;
}

static void expt_remove(struct pci_dev *dev)
{
    /* TODO 12: Get the dev_priv pointer from private data area */
    /* TODO 13: Unmap the pci region */
	printk(KERN_INFO "PCI device memory unmapped\n");

    /* TODO 14: Release the pci regions */
	printk(KERN_INFO "PCI device regions released\n");

    /* TODO 15: Disable the PCI device */
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
