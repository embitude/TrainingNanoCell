#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/rtnetlink.h>
#include <linux/net_tstamp.h>
#include <net/rtnetlink.h>
#include <linux/u64_stats_sync.h>

#define DRV_NAME	"dummy"
#define DRV_VERSION	"1.0"

struct net_device *dev_dummy;

/* fake multicast ability */
static void set_multicast_list(struct net_device *dev)
{
}

struct pcpu_dstats {
	u64			tx_packets;
	u64			tx_bytes;
	struct u64_stats_sync	syncp;
};

static void dummy_get_stats64(struct net_device *dev,
			      struct rtnl_link_stats64 *stats)
{
	int i;
	printk("Getting stats\n");

	for_each_possible_cpu(i) {
		const struct pcpu_dstats *dstats;
		u64 tbytes, tpackets;
		unsigned int start;

		dstats = per_cpu_ptr(dev->dstats, i);
		do {
			start = u64_stats_fetch_begin_irq(&dstats->syncp);
			tbytes = dstats->tx_bytes;
			tpackets = dstats->tx_packets;
		} while (u64_stats_fetch_retry_irq(&dstats->syncp, start));
		stats->tx_bytes += tbytes;
		stats->tx_packets += tpackets;
	}
}

static netdev_tx_t dummy_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct pcpu_dstats *dstats = this_cpu_ptr(dev->dstats);

	u64_stats_update_begin(&dstats->syncp);
	//TODO 8: Update the packet count (tx_packets, tx_bytes
	u64_stats_update_end(&dstats->syncp);

	skb_tx_timestamp(skb);
	//TODO 9: Free the skb buffer
	return NETDEV_TX_OK;
}

static int dummy_dev_init(struct net_device *dev)
{
	dev->dstats = netdev_alloc_pcpu_stats(struct pcpu_dstats);
	if (!dev->dstats)
		return -ENOMEM;

	return 0;
}

static void dummy_dev_uninit(struct net_device *dev)
{
	free_percpu(dev->dstats);
}

static int dummy_change_carrier(struct net_device *dev, bool new_carrier)
{
	//TODO 7: Handle carrier change event
	return 0;
}

//TODO 5: Populate the netdev operations
static const struct net_device_ops dummy_netdev_ops = {
};

static void dummy_setup(struct net_device *dev)
{
	printk("Setting up\n");
	ether_setup(dev);

	/* Initialize the device structure. */
	//TODO 6: Initialize the netdev_ops file 
	dev->needs_free_netdev = true;

	/* Fill in device structure with ethernet-generic values. */
	dev->flags |= IFF_NOARP;
	dev->flags &= ~IFF_MULTICAST;
	dev->priv_flags |= IFF_LIVE_ADDR_CHANGE | IFF_NO_QUEUE;
	dev->features	|= NETIF_F_SG | NETIF_F_FRAGLIST;
	dev->features	|= NETIF_F_ALL_TSO;
	dev->features	|= NETIF_F_HW_CSUM | NETIF_F_HIGHDMA | NETIF_F_LLTX;
	dev->features	|= NETIF_F_GSO_ENCAP_ALL;
	dev->hw_features |= dev->features;
	dev->hw_enc_features |= dev->features;
	eth_hw_addr_random(dev);

	dev->min_mtu = 0;
	dev->max_mtu = 0;
}

static int __init dummy_init_module(void)
{
	int err;
	printk("Dummy link one called\n");

	//TODO 1: Allocate the netdev structure and assign to dev_dummy
	if (!dev_dummy)
		return -ENOMEM;
	//TODO 2: Register the netdevice
	if (err < 0)
		goto err;

	return 0;

err:
	//TODO 3: De-allocate the netdevice
	return err;
}

static void __exit dummy_cleanup_module(void)
{
	//TODO 4: Unregister the netdevice
}

module_init(dummy_init_module);
module_exit(dummy_cleanup_module);
MODULE_LICENSE("GPL");
MODULE_ALIAS_RTNL_LINK(DRV_NAME);
MODULE_VERSION(DRV_VERSION);
