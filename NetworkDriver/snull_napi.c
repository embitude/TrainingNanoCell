/*
 * The poll implementation.
 */
static int snull_poll(struct napi_struct *napi, int budget)
{
	int npackets = 0;
	struct sk_buff *skb;
	struct snull_priv *priv = container_of(napi, struct snull_priv, napi);
	struct net_device *dev = priv->dev;
	struct snull_packet *pkt;
    
	while (npackets < budget && priv->rx_queue) {
		//TODO 1: Dequeue the packet from the rx queue
		//TODO 2: Allocate the skb
		if (! skb) {
			if (printk_ratelimit())
				printk(KERN_NOTICE "snull: packet dropped\n");
			priv->stats.rx_dropped++;
			npackets++;
			snull_release_buffer(pkt);
			continue;
		}
		//TODO 3: Align to the 16B boundardy and copy the data
		skb->dev = dev;
		skb->protocol = eth_type_trans(skb, dev);
		skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
		//TODO 4: Push the skb to the above layer
		
        	/* Maintain stats */
		npackets++;
		priv->stats.rx_packets++;
		priv->stats.rx_bytes += pkt->datalen;
		//TODO 5: Release the buffer
		snull_release_buffer(pkt);
	}
	/* If we processed all packets, we're done; tell the kernel and reenable ints */
	if (npackets < budget) {
		unsigned long flags;
		spin_lock_irqsave(&priv->lock, flags);
		//TODO 6: Disable the interrupts if NAPI is completed
		spin_unlock_irqrestore(&priv->lock, flags);
	}
	return npackets;
}

/*
 * A NAPI interrupt handler.
 */
static void snull_napi_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int statusword;
	struct snull_priv *priv;

	/*
	 * As usual, check the "device" pointer for shared handlers.
	 * Then assign "struct device *dev"
	 */
	struct net_device *dev = (struct net_device *)dev_id;
	/* ... and check with hw if it's really ours */

	/* paranoid */
	if (!dev)
		return;

	/* Lock the device */
	priv = netdev_priv(dev);
	spin_lock(&priv->lock);

	/* retrieve statusword: real netdevices use I/O instructions */
	statusword = priv->status;
	priv->status = 0;
	if (statusword & SNULL_RX_INTR) {
		//TODO 7: Disable further interrupts & trigger the polling
	}
	if (statusword & SNULL_TX_INTR) {
        	/* a transmission is over: free the skb */
		priv->stats.tx_packets++;
		priv->stats.tx_bytes += priv->tx_packetlen;
		if(priv->skb) {
			//TODO 8: Free up the SKB
			priv->skb = 0;
		}
	}

	/* Unlock the device and we are done */
	spin_unlock(&priv->lock);
	return;
}

