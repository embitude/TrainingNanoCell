#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/errno.h>

#include <asm/io.h>

/* TODO 1: Update the Vendor ID & Product ID as per the device */
#define EXPT_VENDOR_ID PCI_VENDOR_ID_REALTEK
#define EXPT_PRODUCT_ID 0x8136 /* Fast ethernet card on PCs */

static void display_pci_config_space(struct pci_dev *dev)
{
	int i;
	uint8_t b;
	uint16_t w;
   	uint32_t dw;
    printk("Config invoked\n");

    /* TODO 7: Print the BARs - range, flag value, IO/MEM, Prefetch */
	for (i = 0; i < 6; i++)
	{
		printk(KERN_INFO "Bar %d: 0x%016llu-%016llu : %08lX : %s %s\n", i,
			i,
			i,
			i,
			" IO""/MEM",
			"PREFETCH" "NON-PREFETCH");
	}

    /* 
     * TODO 8: Read the Configuration space 
     * Vendor ID, Device ID, Revision ID, Class code, Header Type
     * BARs
     */
	printk(KERN_INFO "PCI Configuration Space:\n");

}

static int expt_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	printk(KERN_INFO "PCI device registered\n");

    /* TODO 6: Display the configuration */
    display_pci_config_space(dev);

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
