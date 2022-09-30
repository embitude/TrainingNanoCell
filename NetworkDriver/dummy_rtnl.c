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



static int dummy_validate(struct nlattr *tb[], struct nlattr *data[],
                          struct netlink_ext_ack *extack)
{
        printk("Validating\n");
        if (tb[IFLA_ADDRESS]) {
                if (nla_len(tb[IFLA_ADDRESS]) != ETH_ALEN)
                        return -EINVAL;
                if (!is_valid_ether_addr(nla_data(tb[IFLA_ADDRESS])))
                        return -EADDRNOTAVAIL;
        }
        return 0;
}

static struct rtnl_link_ops dummy_link_ops __read_mostly = {
        .kind           = DRV_NAME,
        .setup          = dummy_setup,
        .validate       = dummy_validate,
};

