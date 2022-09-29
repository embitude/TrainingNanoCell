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

/* TODO 16: Interrupt Mask & Status Register */
#define	INTR_MASK_REG 0
#define	INTR_STATUS_REG 0

/* TODO 17: Define PHY_STATUS_REG  and PHY_LINK_OK_BIT */
#define	PHY_STATUS_REG 0
#define PHY_LINK_OK_BIT 0

/* TODO 18: Define the link change interrupt */
#define INTR_LINK_CHG_BIT 0

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

static inline void hw_enable_intr(struct pci_dev *dev)
{
	struct dev_priv *dpv = pci_get_drvdata(dev);
    /* TODO 22: Enable link change interrupt */
}

static inline void hw_disable_intr(struct pci_dev *dev)
{
	struct dev_priv *dpv = pci_get_drvdata(dev);

    /* TODO 23: Enable link change interrupt */
}

static inline void hw_init(struct pci_dev *dev)
{
	hw_enable_intr(dev);
}

static inline void hw_shut(struct pci_dev *dev)
{
	hw_disable_intr(dev);
}

static irqreturn_t expt_pci_intr_handler(int irq, void *dev)
{

	struct dev_priv *dpv = pci_get_drvdata(dev);
	int cur_state;
    printk(KERN_INFO "Link Change interrupt received\n");

    /* TODO 27: Check INTR_STATUS_REG to check if its Link change Interrupt */
	if (0) // Not our interrupt
	{
		printk(KERN_ERR "Expt Intr: Not our interrupt (Should not happen)\n");
		return IRQ_NONE;
	}

    /* TODO 28: Clear the link change interrupt in interrupt status register */
    /* TODO 29: Read the PHY_LINK_OK_BIT to check the link status */
    /* TODO 30: As per the cur_state, display link up or down */
    if (cur_state == 1)
		printk(KERN_INFO "Expt Intr: link up\n");
    else
		printk(KERN_INFO "Expt Intr: link down\n");
	return IRQ_HANDLED;
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

    /* TODO 19: Enable msi interrupts */
	if (retval)
	{
		printk(KERN_INFO "Unable to enable MSI for this PCI device\n"); // Still okay to continue
	}
	else
	{
		printk(KERN_INFO "PCI device enabled w/ MSI\n");
		printk(KERN_INFO "IRQ w/ MSI: %u\n", dev->irq);
	}

    /* TODO 20: Register interrupt handler */
	if (retval)
	{
		printk(KERN_ERR "Unable to register interrupt handler for this PCI device\n");
		if (pci_dev_msi_enabled(dev)) pci_disable_msi(dev);
		iounmap(dpv->reg_base);
		pci_release_regions(dev);
		pci_disable_device(dev);
		return retval;
	}
	else
	{
		printk(KERN_INFO "PCI device interrupt handler registered\n");
	}
    /* TODO 21: Initialize the hardware */

	return 0;
}

static void expt_remove(struct pci_dev *dev)
{
    /* TODO 12: Get the dev_priv pointer from private data area */
	struct dev_priv *dpv = pci_get_drvdata(dev);

    /* TODO 24: Shut the hardware (Disable interrupts)  */
	pci_set_drvdata(dev, NULL);

    /* TODO 25: Free up the IRQ */
	printk(KERN_INFO "PCI device interrupt handler unregistered\n");

    /* TODO 26: Disable the MSI, if it's enabled */
	printk(KERN_INFO "PCI device MSI disabled\n");

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
