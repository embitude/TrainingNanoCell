#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/errno.h>

#include <asm/io.h>

/* TODO 1: Update the Vendor ID & Product ID as per the device */
#define EXPT_VENDOR_ID 0
#define EXPT_PRODUCT_ID 0

static int expt_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	printk(KERN_INFO "PCI device registered\n");

	return 0;
}

static void expt_remove(struct pci_dev *dev)
{
	printk(KERN_INFO "PCI device unregistered\n");
}

/* TODO 2: Populate the pci_device_id table, use PCI_DEVICE */
/* Table of devices that work with this driver */
static struct pci_device_id expt_table[] =
{
	{
	},
	{} /* Terminating entry */
};
MODULE_DEVICE_TABLE (pci, expt_table);

/* TODO 3: Populate the pci driver fields - .name, id_table, .probe, .remove */
static struct pci_driver pci_drv =
{
};

static int __init expt_pci_init(void)
{
	int result;

	/* Register this driver with the PCI subsystem */
    /* TODO 4: Register the PCI driver */
	if ((result))
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
	printk(KERN_INFO "Expt PCI driver unregistered\n");
}

module_init(expt_pci_init);
module_exit(expt_pci_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pradeep Tewani");
MODULE_DESCRIPTION("Experimental PCI Device Driver");
