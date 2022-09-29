#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/errno.h>

#include <asm/io.h>
#include <linux/mii.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include <linux/delay.h>

#define	MAC_ADDR_REG_START 0

/* TODO 16: Interrupt Mask & Status Register */
#define	INTR_MASK_REG 0x3C
#define	INTR_STATUS_REG 0x3E

/* TODO 17: Define PHY_STATUS_REG  and PHY_LINK_OK_BIT */
#define	PHY_STATUS_REG 0x6C
#define PHY_LINK_OK_BIT (1 << 1)

/* TODO 18: Define the link change interrupt */
#define INTR_LINK_CHG_BIT (1 << 5)

/* TODO 2.1 Define the below macros for DMA */
#define TX_DESC_ADDR_LOW_REG 0
#define TX_DESC_ADDR_HIGH_REG 0
#define CHIP_CMD_REG 0
#define TX_POLL_REG 0
#define TX_CONFIG_REG 0
#define RX_CONFIG_REG 0
#define	CFG9346_REG 0
#define RX_MAX_SIZE_REG 0
#define RX_DESC_ADDR_LOW_REG 0
#define RX_DESC_ADDR_HIGH_REG 0

/* TODO 2.2 Define below macros for interrupts */
#define INTR_TX_DESC_UNAVAIL 0
#define INTR_RX_DESC_UNAVAIL 0
#define INTR_TX_ERR 0
#define INTR_TX_OK_BIT 0
#define INTR_RX_ERR 0
#define INTR_RX_OK_BIT 0

/* TODO 2.3 Define below macros for TX_OK, LINK_CHANGE & INTR_RX_OK */
#define	INTR_MASK 0
#define	INTR_ERR_MASK (INTR_TX_DESC_UNAVAIL | INTR_RX_DESC_UNAVAIL | INTR_TX_ERR | INTR_RX_ERR)

/* TODO 2.4 Define Chip commands */
/* CHIP_CMD cmds */
#define CMD_RESET 0
#define CMD_RX_ENB 0
#define CMD_TX_ENB 0

/* TX_POLL_REG (Transmit Priority Polling) bits */
#define HPQ_BIT (1 << 7)
#define NPQ_BIT (1 << 6)

/* CFG9346_REG cmds */
#define CFG9346_LOCK 0x00
#define CFG9346_UNLOCK 0xC0

#define PKT_SIZE 256
#define TX_PKT_SIZE PKT_SIZE
#define RX_PKT_SIZE PKT_SIZE

/* TODO 2.5 Populate Descriptors - opts1, opts2 and addr */
typedef struct _Desc
{
} Desc;

/* TODO 2.6 Define descriptor status bits */
enum DescStatusBits {
	DescOwn		= 0, /* Descriptor is owned by NIC */
	RingEnd		= 0, /* End of descriptor ring */
	FirstFrag	= 0, /* First segment of a packet */
	LastFrag	= 0, /* Final segment of a packet */
};

/* TODO 1: Update the Vendor ID & Product ID as per the device */
#define EXPT_VENDOR_ID PCI_VENDOR_ID_REALTEK
#define EXPT_PRODUCT_ID 0x8136 /* Fast ethernet card on PCs */

static struct dev_priv
{
	void __iomem *reg_base;
	dev_t dev;
	struct cdev c_dev;
	struct class *cl;
    /* DMA related fields */
	Desc *desc_tx, *desc_rx;
	dma_addr_t phy_desc_tx, phy_desc_rx;
	uint8_t *pkt_tx, *pkt_rx;
	dma_addr_t phy_pkt_tx, phy_pkt_rx;
	int rx_pkt_size;
	atomic_t pkt_tx_busy, pkt_rx_avail;
} _pvt;

static int expt_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Expt: In open\n");
	f->private_data = &_pvt;
	return 0;
}
static int expt_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Expt: In close\n");
	return 0;
}

static ssize_t expt_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	struct dev_priv *dpv = f->private_data;
	int i, to_read;
	uint8_t addr;

	printk(KERN_INFO "Expt: In read - Buf: %p; Len: %d; Off: %Ld\nData:\n", buf, len, *off);

	if (*off >= 6)
		return 0;

	to_read = min(len, 6 - (size_t)*off);

	for (i = 0; i < to_read; i++)
	{
        /* TODO 13: Read the mac address ioread8 & send it to user space */
		addr = ioread8(dpv->reg_base + MAC_ADDR_REG_START + *off + i);
		if (copy_to_user(buf + i, &addr, 1))
		{
			return -EFAULT;
		}
	}
	*off += to_read;

	return to_read;
}

static ssize_t expt_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	struct dev_priv *dpv = f->private_data;
	int to_write;

	printk(KERN_INFO "Expt: In write\n");
    /* TODO 2.29: Atomically increment the tx_busy */
	if (atomic_read(&dpv->pkt_tx_busy) != 1)
	{
		atomic_dec(&dpv->pkt_tx_busy);
		return -EBUSY;
	}

	to_write = min(len, (size_t)TX_PKT_SIZE);

	if (copy_from_user(dpv->pkt_tx, buf, to_write))
	{
		return -EFAULT;
	}

	/* 
     * TODO 2.30: Make the buffer available to device.
     * For this, set the correspond bit in tx descriptor opts1
     * and update the write size as well
     * Make sure to use le32_to_cpu before setting
     */
    /* TODO 2.31: Enable the Transmit with NPQ_BIT */

	return len;
}

static struct file_operations fops =
{
	.open = expt_open,
	.release = expt_close,
	.read = expt_read,
	.write = expt_write,
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

static int setup_buffers(struct pci_dev *dev)
{
	struct dev_priv *dpv = pci_get_drvdata(dev);

    /* TODO 2.8: Allocate the coherent memory for tx descriptor */
	if (!dpv->desc_tx)
		return -ENOMEM;
    /* TODO 2.9: Allocate the coherent memory for rx descriptor */
	if (!dpv->desc_rx)
	{
		pci_free_consistent(dev, sizeof(Desc), dpv->desc_tx, dpv->phy_desc_tx);
		return -ENOMEM;
	}

    /* TODO 2.10: Allocate the buffer of size TX_PKT_SIZE using kmalloc.
     * Uset GFP_KERNEL and GFP_DMA flags
     * */
	if (dpv->pkt_tx == NULL)
	{
		pci_free_consistent(dev, sizeof(Desc), dpv->desc_rx, dpv->phy_desc_rx);
		pci_free_consistent(dev, sizeof(Desc), dpv->desc_tx, dpv->phy_desc_tx);
	}
    /* TODO 2.11A : Map transmit buffer for performing the dma */
    /* TODO 2.11B: Map receive buffer for performing the dma */
	if (dpv->pkt_rx  == NULL)
	{
		pci_unmap_single(dev, dpv->phy_pkt_tx, TX_PKT_SIZE, PCI_DMA_TODEVICE);
		kfree(dpv->pkt_tx);
		pci_free_consistent(dev, sizeof(Desc), dpv->desc_rx, dpv->phy_desc_rx);
		pci_free_consistent(dev, sizeof(Desc), dpv->desc_tx, dpv->phy_desc_tx);
	}
    /* TODO 2.12: Map receive buffer for performing the dma */
    /* 
     * TODO 2.13: Initalize opts1 for Tx descriptor
     * Use cpu_to_le32 for assignment.  
     */
    /* TODO 2.14: Initialize the addr for Tx descriptor. use cpu_to_le64 */
    /* 
     * TODO 2.15: Initalize opts1 for Tx descriptor
     * Use cpu_to_le32 for assignment.  
     */
    /* TODO 2.16: Initialize the addr for Rx descriptor. use cpu_to_le64 */

	iowrite8(CFG9346_UNLOCK, dpv->reg_base + CFG9346_REG);
    /* 
     * TODO 2.17: Update the TX_DESC_ADDR_LOW_REG and TX_DESC_ADDR_HIGH_REG
     * with physical address of TX Descriptor
     * Use DMA_BIT_MASK(32) for masking the upper 32 bits
     */
	iowrite32(dpv->phy_desc_rx & DMA_BIT_MASK(32), dpv->reg_base + RX_DESC_ADDR_LOW_REG);
	iowrite32((uint64_t)(dpv->phy_desc_rx) >> 32, dpv->reg_base + RX_DESC_ADDR_HIGH_REG);
	iowrite8(CFG9346_LOCK, dpv->reg_base + CFG9346_REG);
	
    /* 
     * TODO 2.18: Atomically clear the pkt_tx_busyand pkt_rx_avail
     * with atomic_set
     */ 

	return 0;
}

static void cleanup_buffers(struct pci_dev *dev)
{
	struct dev_priv *dpv = pci_get_drvdata(dev);

    /* TODO 2.19: unmap the rx buffer */
    /* TODO 2.20: Free up the rx buffer */
    /* TODO 2.21: unmap the tx buffer */
	kfree(dpv->pkt_tx);

    /* TODO 2.22: Free up the rx and tx Descriptor */
}

static inline void hw_enable_intr(struct dev_priv *dpv, uint16_t mask)
{
	iowrite16(ioread16(dpv->reg_base + INTR_MASK_REG) | mask, dpv->reg_base + INTR_MASK_REG);
}

static inline void hw_disable_intr(struct dev_priv *dpv, uint16_t mask)
{
	iowrite16(ioread16(dpv->reg_base + INTR_MASK_REG) & ~mask, dpv->reg_base + INTR_MASK_REG);
}

static inline void hw_init(struct pci_dev *dev)
{
	struct dev_priv *dpv = pci_get_drvdata(dev);
    /* TODO 2.24: Zero out the TX_CONFIG_REG */
    /* TODO 2.25: Enable RX and TX by setting the chip command register */
    /* Enable the link change, TX_OK and RX_OK & Err mask insterrupt */
	hw_enable_intr(dpv, INTR_MASK | INTR_ERR_MASK);
}

static inline void hw_shut(struct pci_dev *dev)
{
    struct dev_priv *dpv = pci_get_drvdata(dev);
	int i;
    /* TODO 2.26: Disable interrupts */

    /* TODO 2.27: Reset the chip to disable Tx and Rx */
	for (i = 0; i < 100; i--)
	{
		udelay(100);
		if ((ioread8(dpv->reg_base + CHIP_CMD_REG) & CMD_RESET) == 0)
			break;
	}
}

static irqreturn_t expt_pci_intr_handler(int irq, void *dev)
{
	struct dev_priv *dpv = pci_get_drvdata(dev);
	int cur_state;
	uint16_t intr_status;

    /* 
     * TODO 2.32: Read the interrupt status register into intr_status
     * and verify if any of enabled interrupts is triggered
     */
	if (intr_status) // Not our interrupt
	{
		printk(KERN_ERR "Expt Intr: Not our interrupt (Should not happen)\n");
		return IRQ_NONE;
	}

    if (intr_status & INTR_LINK_CHG_BIT)
    {
        /* TODO 28: Clear the link change interrupt in interrupt status register */
        printk(KERN_INFO "Link Change interrupt received\n");
        iowrite16(INTR_LINK_CHG_BIT, dpv->reg_base + INTR_STATUS_REG); // Clear it off
        /* TODO 29: Read the PHY_LINK_OK_BIT to check the link status */
        cur_state = ((ioread8(dpv->reg_base + PHY_STATUS_REG) & PHY_LINK_OK_BIT) ? 1 : 0);
        /* TODO 30: As per the cur_state, display link up or down */
        if (cur_state == 1)
            printk(KERN_INFO "Expt Intr: link up\n");
        else
            printk(KERN_INFO "Expt Intr: link down\n");
    }
    /* TODO 2.33: Check if the Transmisson was successful */
    if (intr_status)
	{
        /* TODO 2.34: Clear the TX_OK bit */
        /* TODO 2.35: Decrement the tx busy atomically */
		printk(KERN_INFO "Expt Intr: packet transmitted\n");
	}
	if (intr_status & INTR_ERR_MASK)
	{
		if (intr_status & INTR_TX_DESC_UNAVAIL)
			printk(KERN_INFO "Expt Intr: tx desc unavailable\n");
		if (intr_status & INTR_RX_DESC_UNAVAIL)
		{
			hw_disable_intr(dpv, INTR_RX_DESC_UNAVAIL);
			// Interrupt shall be enabled in read once desc is available
			printk(KERN_INFO "Expt Intr: rx desc unavailable\n");
		}
		if (intr_status & INTR_TX_ERR)
			printk(KERN_INFO "Expt Intr: tx error\n");
		if (intr_status & INTR_RX_ERR)
			printk(KERN_INFO "Expt Intr: rx error\n");
		iowrite16(INTR_ERR_MASK, dpv->reg_base + INTR_STATUS_REG); // Clear it off
	}

	return IRQ_HANDLED;
}

static int expt_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    int retval;
    /* TODO 6: Define the private data structure */
	struct dev_priv *dpv = &_pvt; // Should be converted to kmalloc for multi-card support

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

	pci_set_master(dev); // Enable the PCI device to become the bus master for initiating DMA

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

    /* TODO 19: Enable msi interrupts */
	retval = pci_enable_msi(dev);
	if (retval)
	{
		printk(KERN_INFO "Unable to enable MSI for this PCI device\n"); // Still okay to continue
	}
	else
	{
		printk(KERN_INFO "PCI device enabled w/ MSI\n");
		printk(KERN_INFO "IRQ w/ MSI: %u\n", dev->irq);
	}

    /* TODO 2.7 Invoke setup_buffers */
	if (retval)
	{
		printk(KERN_ERR "Unable to setup buffers for this PCI device\n");
		if (pci_dev_msi_enabled(dev)) pci_disable_msi(dev);
		pci_set_drvdata(dev, NULL);
		iounmap(dpv->reg_base);
		pci_release_regions(dev);
		pci_disable_device(dev);
		return retval;
	}
	else
	{
		printk(KERN_INFO "PCI device buffers setup\n");
	}
    /* TODO 20: Register interrupt handler */
	retval = request_irq(dev->irq, expt_pci_intr_handler,
							(pci_dev_msi_enabled(dev) ? 0 : IRQF_SHARED), "expt_pci", dev);
	if (retval)
	{
		printk(KERN_ERR "Unable to register interrupt handler for this PCI device\n");
		cleanup_buffers(dev);
		if (pci_dev_msi_enabled(dev)) pci_disable_msi(dev);
		pci_set_drvdata(dev, NULL);
		iounmap(dpv->reg_base);
		pci_release_regions(dev);
		pci_disable_device(dev);
		return retval;
	}
	else
	{
		printk(KERN_INFO "PCI device interrupt handler registered\n");
	}
    /* TODO 2.23: Initialize the hardware */

	retval = char_register_dev(dpv);
	if (retval)
	{
		/* Something prevented us from registering this driver */
		printk(KERN_ERR "Unable to register the character vertical\n");
		hw_shut(dev); // Needs the drvdata
		free_irq(dev->irq, dev);
		cleanup_buffers(dev);
		if (pci_dev_msi_enabled(dev)) pci_disable_msi(dev);
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
	printk(KERN_INFO "PCI device registered\n");

	return 0;
}

static void expt_remove(struct pci_dev *dev)
{
    /* TODO 12: Get the dev_priv pointer from private data area */
	struct dev_priv *dpv = pci_get_drvdata(dev);

    /* TODO 2.28: Shut the hardware (Disable interrupts)  */
	pci_set_drvdata(dev, NULL);

	char_deregister_dev(dpv);
	printk(KERN_INFO "Character vertical deregistered\n");

    /* TODO 25: Free up the IRQ */
	free_irq(dev->irq, dev);
	printk(KERN_INFO "PCI device interrupt handler unregistered\n");

    /* TODO 26: Disable the MSI, if it's enabled */
	if (pci_dev_msi_enabled(dev)) pci_disable_msi(dev);
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
