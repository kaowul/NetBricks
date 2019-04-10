/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2017 Intel Corporation
 */

#ifndef _RTE_ETHDEV_H_
#define _RTE_ETHDEV_H_

/**
 * @file
 *
 * RTE Ethernet Device API
 *
 * The Ethernet Device API is composed of two parts:
 *
 * - The application-oriented Ethernet API that includes functions to setup
 *   an Ethernet device (configure it, setup its RX and TX queues and start it),
 *   to get its MAC address, the speed and the status of its physical link,
 *   to receive and to transmit packets, and so on.
 *
 * - The driver-oriented Ethernet API that exports functions allowing
 *   an Ethernet Poll Mode Driver (PMD) to allocate an Ethernet device instance,
 *   create memzone for HW rings and process registered callbacks, and so on.
 *   PMDs should include rte_ethdev_driver.h instead of this header.
 *
 * By default, all the functions of the Ethernet Device API exported by a PMD
 * are lock-free functions which assume to not be invoked in parallel on
 * different logical cores to work on the same target object.  For instance,
 * the receive function of a PMD cannot be invoked in parallel on two logical
 * cores to poll the same RX queue [of the same port]. Of course, this function
 * can be invoked in parallel by different logical cores on different RX queues.
 * It is the responsibility of the upper level application to enforce this rule.
 *
 * If needed, parallel accesses by multiple logical cores to shared queues
 * shall be explicitly protected by dedicated inline lock-aware functions
 * built on top of their corresponding lock-free functions of the PMD API.
 *
 * In all functions of the Ethernet API, the Ethernet device is
 * designated by an integer >= 0 named the device port identifier.
 *
 * At the Ethernet driver level, Ethernet devices are represented by a generic
 * data structure of type *rte_eth_dev*.
 *
 * Ethernet devices are dynamically registered during the PCI probing phase
 * performed at EAL initialization time.
 * When an Ethernet device is being probed, an *rte_eth_dev* structure and
 * a new port identifier are allocated for that device. Then, the eth_dev_init()
 * function supplied by the Ethernet driver matching the probed PCI
 * device is invoked to properly initialize the device.
 *
 * The role of the device init function consists of resetting the hardware,
 * checking access to Non-volatile Memory (NVM), reading the MAC address
 * from NVM etc.
 *
 * If the device init operation is successful, the correspondence between
 * the port identifier assigned to the new device and its associated
 * *rte_eth_dev* structure is effectively registered.
 * Otherwise, both the *rte_eth_dev* structure and the port identifier are
 * freed.
 *
 * The functions exported by the application Ethernet API to setup a device
 * designated by its port identifier must be invoked in the following order:
 *     - rte_eth_dev_configure()
 *     - rte_eth_tx_queue_setup()
 *     - rte_eth_rx_queue_setup()
 *     - rte_eth_dev_start()
 *
 * Then, the network application can invoke, in any order, the functions
 * exported by the Ethernet API to get the MAC address of a given device, to
 * get the speed and the status of a device physical link, to receive/transmit
 * [burst of] packets, and so on.
 *
 * If the application wants to change the configuration (i.e. call
 * rte_eth_dev_configure(), rte_eth_tx_queue_setup(), or
 * rte_eth_rx_queue_setup()), it must call rte_eth_dev_stop() first to stop the
 * device and then do the reconfiguration before calling rte_eth_dev_start()
 * again. The transmit and receive functions should not be invoked when the
 * device is stopped.
 *
 * Please note that some configuration is not stored between calls to
 * rte_eth_dev_stop()/rte_eth_dev_start(). The following configuration will
 * be retained:
 *
 *     - flow control settings
 *     - receive mode configuration (promiscuous mode, hardware checksum mode,
 *       RSS/VMDQ settings etc.)
 *     - VLAN filtering configuration
 *     - MAC addresses supplied to MAC address array
 *     - flow director filtering mode (but not filtering rules)
 *     - NIC queue statistics mappings
 *
 * Any other configuration will not be stored and will need to be re-entered
 * before a call to rte_eth_dev_start().
 *
 * Finally, a network application can close an Ethernet device by invoking the
 * rte_eth_dev_close() function.
 *
 * Each function of the application Ethernet API invokes a specific function
 * of the PMD that controls the target device designated by its port
 * identifier.
 * For this purpose, all device-specific functions of an Ethernet driver are
 * supplied through a set of pointers contained in a generic structure of type
 * *eth_dev_ops*.
 * The address of the *eth_dev_ops* structure is stored in the *rte_eth_dev*
 * structure by the device init function of the Ethernet driver, which is
 * invoked during the PCI probing phase, as explained earlier.
 *
 * In other words, each function of the Ethernet API simply retrieves the
 * *rte_eth_dev* structure associated with the device port identifier and
 * performs an indirect invocation of the corresponding driver function
 * supplied in the *eth_dev_ops* structure of the *rte_eth_dev* structure.
 *
 * For performance reasons, the address of the burst-oriented RX and TX
 * functions of the Ethernet driver are not contained in the *eth_dev_ops*
 * structure. Instead, they are directly stored at the beginning of the
 * *rte_eth_dev* structure to avoid an extra indirect memory access during
 * their invocation.
 *
 * RTE ethernet device drivers do not use interrupts for transmitting or
 * receiving. Instead, Ethernet drivers export Poll-Mode receive and transmit
 * functions to applications.
 * Both receive and transmit functions are packet-burst oriented to minimize
 * their cost per packet through the following optimizations:
 *
 * - Sharing among multiple packets the incompressible cost of the
 *   invocation of receive/transmit functions.
 *
 * - Enabling receive/transmit functions to take advantage of burst-oriented
 *   hardware features (L1 cache, prefetch instructions, NIC head/tail
 *   registers) to minimize the number of CPU cycles per packet, for instance,
 *   by avoiding useless read memory accesses to ring descriptors, or by
 *   systematically using arrays of pointers that exactly fit L1 cache line
 *   boundaries and sizes.
 *
 * The burst-oriented receive function does not provide any error notification,
 * to avoid the corresponding overhead. As a hint, the upper-level application
 * might check the status of the device link once being systematically returned
 * a 0 value by the receive function of the driver for a given number of tries.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Use this macro to check if LRO API is supported */
#define RTE_ETHDEV_HAS_LRO_SUPPORT
/*
#include <rte_compat.h>
#include <rte_log.h>
#include <rte_interrupts.h>
#include <rte_dev.h>
#include <rte_devargs.h>
#include <rte_errno.h>
#include <rte_common.h>
#include <rte_config.h>

#include "rte_ether.h"
#include "rte_eth_ctrl.h"
#include "rte_dev_info.h"
*/
#include "rte_config.h"
#include "rte_eth_ctrl.h"

extern int rte_eth_dev_logtype;

#define RTE_ETHDEV_LOG(level, ...) \
	rte_log(RTE_LOG_ ## level, rte_eth_dev_logtype, "" __VA_ARGS__)

struct rte_mbuf;

/**
 * A structure used to retrieve statistics for an Ethernet port.
 * Not all statistics fields in struct rte_eth_stats are supported
 * by any type of network interface card (NIC). If any statistics
 * field is not supported, its value is 0.
 */
struct rte_eth_stats {
	uint64_t ipackets;  /**< Total number of successfully received packets. */
	uint64_t opackets;  /**< Total number of successfully transmitted packets.*/
	uint64_t ibytes;    /**< Total number of successfully received bytes. */
	uint64_t obytes;    /**< Total number of successfully transmitted bytes. */
	uint64_t imissed;
	/**< Total of RX packets dropped by the HW,
	 * because there are no available buffer (i.e. RX queues are full).
	 */
	uint64_t ierrors;   /**< Total number of erroneous received packets. */
	uint64_t oerrors;   /**< Total number of failed transmitted packets. */
	uint64_t rx_nombuf; /**< Total number of RX mbuf allocation failures. */
	uint64_t q_ipackets[RTE_ETHDEV_QUEUE_STAT_CNTRS];
	/**< Total number of queue RX packets. */
	uint64_t q_opackets[RTE_ETHDEV_QUEUE_STAT_CNTRS];
	/**< Total number of queue TX packets. */
	uint64_t q_ibytes[RTE_ETHDEV_QUEUE_STAT_CNTRS];
	/**< Total number of successfully received queue bytes. */
	uint64_t q_obytes[RTE_ETHDEV_QUEUE_STAT_CNTRS];
	/**< Total number of successfully transmitted queue bytes. */
	uint64_t q_errors[RTE_ETHDEV_QUEUE_STAT_CNTRS];
	/**< Total number of queue packets received that are dropped. */
};

/**
 * Device supported speeds bitmap flags
 */
#define ETH_LINK_SPEED_AUTONEG  (0 <<  0)  /**< Autonegotiate (all speeds) */
#define ETH_LINK_SPEED_FIXED    (1 <<  0)  /**< Disable autoneg (fixed speed) */
#define ETH_LINK_SPEED_10M_HD   (1 <<  1)  /**<  10 Mbps half-duplex */
#define ETH_LINK_SPEED_10M      (1 <<  2)  /**<  10 Mbps full-duplex */
#define ETH_LINK_SPEED_100M_HD  (1 <<  3)  /**< 100 Mbps half-duplex */
#define ETH_LINK_SPEED_100M     (1 <<  4)  /**< 100 Mbps full-duplex */
#define ETH_LINK_SPEED_1G       (1 <<  5)  /**<   1 Gbps */
#define ETH_LINK_SPEED_2_5G     (1 <<  6)  /**< 2.5 Gbps */
#define ETH_LINK_SPEED_5G       (1 <<  7)  /**<   5 Gbps */
#define ETH_LINK_SPEED_10G      (1 <<  8)  /**<  10 Gbps */
#define ETH_LINK_SPEED_20G      (1 <<  9)  /**<  20 Gbps */
#define ETH_LINK_SPEED_25G      (1 << 10)  /**<  25 Gbps */
#define ETH_LINK_SPEED_40G      (1 << 11)  /**<  40 Gbps */
#define ETH_LINK_SPEED_50G      (1 << 12)  /**<  50 Gbps */
#define ETH_LINK_SPEED_56G      (1 << 13)  /**<  56 Gbps */
#define ETH_LINK_SPEED_100G     (1 << 14)  /**< 100 Gbps */

/**
 * Ethernet numeric link speeds in Mbps
 */
#define ETH_SPEED_NUM_NONE         0 /**< Not defined */
#define ETH_SPEED_NUM_10M         10 /**<  10 Mbps */
#define ETH_SPEED_NUM_100M       100 /**< 100 Mbps */
#define ETH_SPEED_NUM_1G        1000 /**<   1 Gbps */
#define ETH_SPEED_NUM_2_5G      2500 /**< 2.5 Gbps */
#define ETH_SPEED_NUM_5G        5000 /**<   5 Gbps */
#define ETH_SPEED_NUM_10G      10000 /**<  10 Gbps */
#define ETH_SPEED_NUM_20G      20000 /**<  20 Gbps */
#define ETH_SPEED_NUM_25G      25000 /**<  25 Gbps */
#define ETH_SPEED_NUM_40G      40000 /**<  40 Gbps */
#define ETH_SPEED_NUM_50G      50000 /**<  50 Gbps */
#define ETH_SPEED_NUM_56G      56000 /**<  56 Gbps */
#define ETH_SPEED_NUM_100G    100000 /**< 100 Gbps */

/**
 * A structure used to retrieve link-level information of an Ethernet port.
 */
__extension__
struct rte_eth_link {
	uint32_t link_speed;        /**< ETH_SPEED_NUM_ */
	uint16_t link_duplex  : 1;  /**< ETH_LINK_[HALF/FULL]_DUPLEX */
	uint16_t link_autoneg : 1;  /**< ETH_LINK_[AUTONEG/FIXED] */
	uint16_t link_status  : 1;  /**< ETH_LINK_[DOWN/UP] */
} __attribute__((aligned(8)));      /**< aligned for atomic64 read/write */

/* Utility constants */
#define ETH_LINK_HALF_DUPLEX 0 /**< Half-duplex connection (see link_duplex). */
#define ETH_LINK_FULL_DUPLEX 1 /**< Full-duplex connection (see link_duplex). */
#define ETH_LINK_DOWN        0 /**< Link is down (see link_status). */
#define ETH_LINK_UP          1 /**< Link is up (see link_status). */
#define ETH_LINK_FIXED       0 /**< No autonegotiation (see link_autoneg). */
#define ETH_LINK_AUTONEG     1 /**< Autonegotiated (see link_autoneg). */

/**
 * A structure used to configure the ring threshold registers of an RX/TX
 * queue for an Ethernet port.
 */
struct rte_eth_thresh {
	uint8_t pthresh; /**< Ring prefetch threshold. */
	uint8_t hthresh; /**< Ring host threshold. */
	uint8_t wthresh; /**< Ring writeback threshold. */
};

/**
 *  Simple flags are used for rte_eth_conf.rxmode.mq_mode.
 */
#define ETH_MQ_RX_RSS_FLAG  0x1
#define ETH_MQ_RX_DCB_FLAG  0x2
#define ETH_MQ_RX_VMDQ_FLAG 0x4

/**
 *  A set of values to identify what method is to be used to route
 *  packets to multiple queues.
 */
enum rte_eth_rx_mq_mode {
	/** None of DCB,RSS or VMDQ mode */
	ETH_MQ_RX_NONE = 0,

	/** For RX side, only RSS is on */
	ETH_MQ_RX_RSS = ETH_MQ_RX_RSS_FLAG,
	/** For RX side,only DCB is on. */
	ETH_MQ_RX_DCB = ETH_MQ_RX_DCB_FLAG,
	/** Both DCB and RSS enable */
	ETH_MQ_RX_DCB_RSS = ETH_MQ_RX_RSS_FLAG | ETH_MQ_RX_DCB_FLAG,

	/** Only VMDQ, no RSS nor DCB */
	ETH_MQ_RX_VMDQ_ONLY = ETH_MQ_RX_VMDQ_FLAG,
	/** RSS mode with VMDQ */
	ETH_MQ_RX_VMDQ_RSS = ETH_MQ_RX_RSS_FLAG | ETH_MQ_RX_VMDQ_FLAG,
	/** Use VMDQ+DCB to route traffic to queues */
	ETH_MQ_RX_VMDQ_DCB = ETH_MQ_RX_VMDQ_FLAG | ETH_MQ_RX_DCB_FLAG,
	/** Enable both VMDQ and DCB in VMDq */
	ETH_MQ_RX_VMDQ_DCB_RSS = ETH_MQ_RX_RSS_FLAG | ETH_MQ_RX_DCB_FLAG |
				 ETH_MQ_RX_VMDQ_FLAG,
};

/**
 * for rx mq mode backward compatible
 */
#define ETH_RSS                       ETH_MQ_RX_RSS
#define VMDQ_DCB                      ETH_MQ_RX_VMDQ_DCB
#define ETH_DCB_RX                    ETH_MQ_RX_DCB

/**
 * A set of values to identify what method is to be used to transmit
 * packets using multi-TCs.
 */
enum rte_eth_tx_mq_mode {
	ETH_MQ_TX_NONE    = 0,  /**< It is in neither DCB nor VT mode. */
	ETH_MQ_TX_DCB,          /**< For TX side,only DCB is on. */
	ETH_MQ_TX_VMDQ_DCB,	/**< For TX side,both DCB and VT is on. */
	ETH_MQ_TX_VMDQ_ONLY,    /**< Only VT on, no DCB */
};

/**
 * for tx mq mode backward compatible
 */
#define ETH_DCB_NONE                ETH_MQ_TX_NONE
#define ETH_VMDQ_DCB_TX             ETH_MQ_TX_VMDQ_DCB
#define ETH_DCB_TX                  ETH_MQ_TX_DCB

/**
 * A structure used to configure the RX features of an Ethernet port.
 */
struct rte_eth_rxmode {
	/** The multi-queue packet distribution mode to be used, e.g. RSS. */
	enum rte_eth_rx_mq_mode mq_mode;
	uint32_t max_rx_pkt_len;  /**< Only used if JUMBO_FRAME enabled. */
	uint16_t split_hdr_size;  /**< hdr buf size (header_split enabled).*/
	/**
	 * Per-port Rx offloads to be set using DEV_RX_OFFLOAD_* flags.
	 * Only offloads set on rx_offload_capa field on rte_eth_dev_info
	 * structure are allowed to be set.
	 */
	uint64_t offloads;
};

/**
 * VLAN types to indicate if it is for single VLAN, inner VLAN or outer VLAN.
 * Note that single VLAN is treated the same as inner VLAN.
 */
enum rte_vlan_type {
	ETH_VLAN_TYPE_UNKNOWN = 0,
	ETH_VLAN_TYPE_INNER, /**< Inner VLAN. */
	ETH_VLAN_TYPE_OUTER, /**< Single VLAN, or outer VLAN. */
	ETH_VLAN_TYPE_MAX,
};

/**
 * A structure used to describe a vlan filter.
 * If the bit corresponding to a VID is set, such VID is on.
 */
struct rte_vlan_filter_conf {
	uint64_t ids[64];
};

/**
 * A structure used to configure the Receive Side Scaling (RSS) feature
 * of an Ethernet port.
 * If not NULL, the *rss_key* pointer of the *rss_conf* structure points
 * to an array holding the RSS key to use for hashing specific header
 * fields of received packets. The length of this array should be indicated
 * by *rss_key_len* below. Otherwise, a default random hash key is used by
 * the device driver.
 *
 * The *rss_key_len* field of the *rss_conf* structure indicates the length
 * in bytes of the array pointed by *rss_key*. To be compatible, this length
 * will be checked in i40e only. Others assume 40 bytes to be used as before.
 *
 * The *rss_hf* field of the *rss_conf* structure indicates the different
 * types of IPv4/IPv6 packets to which the RSS hashing must be applied.
 * Supplying an *rss_hf* equal to zero disables the RSS feature.
 */
struct rte_eth_rss_conf {
	uint8_t *rss_key;    /**< If not NULL, 40-byte hash key. */
	uint8_t rss_key_len; /**< hash key length in bytes. */
	uint64_t rss_hf;     /**< Hash functions to apply - see below. */
};

/*
 * The RSS offload types are defined based on flow types which are defined
 * in rte_eth_ctrl.h. Different NIC hardwares may support different RSS offload
 * types. The supported flow types or RSS offload types can be queried by
 * rte_eth_dev_info_get().
 */
#define ETH_RSS_IPV4               (1ULL << RTE_ETH_FLOW_IPV4)
#define ETH_RSS_FRAG_IPV4          (1ULL << RTE_ETH_FLOW_FRAG_IPV4)
#define ETH_RSS_NONFRAG_IPV4_TCP   (1ULL << RTE_ETH_FLOW_NONFRAG_IPV4_TCP)
#define ETH_RSS_NONFRAG_IPV4_UDP   (1ULL << RTE_ETH_FLOW_NONFRAG_IPV4_UDP)
#define ETH_RSS_NONFRAG_IPV4_SCTP  (1ULL << RTE_ETH_FLOW_NONFRAG_IPV4_SCTP)
#define ETH_RSS_NONFRAG_IPV4_OTHER (1ULL << RTE_ETH_FLOW_NONFRAG_IPV4_OTHER)
#define ETH_RSS_IPV6               (1ULL << RTE_ETH_FLOW_IPV6)
#define ETH_RSS_FRAG_IPV6          (1ULL << RTE_ETH_FLOW_FRAG_IPV6)
#define ETH_RSS_NONFRAG_IPV6_TCP   (1ULL << RTE_ETH_FLOW_NONFRAG_IPV6_TCP)
#define ETH_RSS_NONFRAG_IPV6_UDP   (1ULL << RTE_ETH_FLOW_NONFRAG_IPV6_UDP)
#define ETH_RSS_NONFRAG_IPV6_SCTP  (1ULL << RTE_ETH_FLOW_NONFRAG_IPV6_SCTP)
#define ETH_RSS_NONFRAG_IPV6_OTHER (1ULL << RTE_ETH_FLOW_NONFRAG_IPV6_OTHER)
#define ETH_RSS_L2_PAYLOAD         (1ULL << RTE_ETH_FLOW_L2_PAYLOAD)
#define ETH_RSS_IPV6_EX            (1ULL << RTE_ETH_FLOW_IPV6_EX)
#define ETH_RSS_IPV6_TCP_EX        (1ULL << RTE_ETH_FLOW_IPV6_TCP_EX)
#define ETH_RSS_IPV6_UDP_EX        (1ULL << RTE_ETH_FLOW_IPV6_UDP_EX)
#define ETH_RSS_PORT               (1ULL << RTE_ETH_FLOW_PORT)
#define ETH_RSS_VXLAN              (1ULL << RTE_ETH_FLOW_VXLAN)
#define ETH_RSS_GENEVE             (1ULL << RTE_ETH_FLOW_GENEVE)
#define ETH_RSS_NVGRE              (1ULL << RTE_ETH_FLOW_NVGRE)

#define ETH_RSS_IP ( \
	ETH_RSS_IPV4 | \
	ETH_RSS_FRAG_IPV4 | \
	ETH_RSS_NONFRAG_IPV4_OTHER | \
	ETH_RSS_IPV6 | \
	ETH_RSS_FRAG_IPV6 | \
	ETH_RSS_NONFRAG_IPV6_OTHER | \
	ETH_RSS_IPV6_EX)

#define ETH_RSS_UDP ( \
	ETH_RSS_NONFRAG_IPV4_UDP | \
	ETH_RSS_NONFRAG_IPV6_UDP | \
	ETH_RSS_IPV6_UDP_EX)

#define ETH_RSS_TCP ( \
	ETH_RSS_NONFRAG_IPV4_TCP | \
	ETH_RSS_NONFRAG_IPV6_TCP | \
	ETH_RSS_IPV6_TCP_EX)

#define ETH_RSS_SCTP ( \
	ETH_RSS_NONFRAG_IPV4_SCTP | \
	ETH_RSS_NONFRAG_IPV6_SCTP)

#define ETH_RSS_TUNNEL ( \
	ETH_RSS_VXLAN  | \
	ETH_RSS_GENEVE | \
	ETH_RSS_NVGRE)

/**< Mask of valid RSS hash protocols */
#define ETH_RSS_PROTO_MASK ( \
	ETH_RSS_IPV4 | \
	ETH_RSS_FRAG_IPV4 | \
	ETH_RSS_NONFRAG_IPV4_TCP | \
	ETH_RSS_NONFRAG_IPV4_UDP | \
	ETH_RSS_NONFRAG_IPV4_SCTP | \
	ETH_RSS_NONFRAG_IPV4_OTHER | \
	ETH_RSS_IPV6 | \
	ETH_RSS_FRAG_IPV6 | \
	ETH_RSS_NONFRAG_IPV6_TCP | \
	ETH_RSS_NONFRAG_IPV6_UDP | \
	ETH_RSS_NONFRAG_IPV6_SCTP | \
	ETH_RSS_NONFRAG_IPV6_OTHER | \
	ETH_RSS_L2_PAYLOAD | \
	ETH_RSS_IPV6_EX | \
	ETH_RSS_IPV6_TCP_EX | \
	ETH_RSS_IPV6_UDP_EX | \
	ETH_RSS_PORT  | \
	ETH_RSS_VXLAN | \
	ETH_RSS_GENEVE | \
	ETH_RSS_NVGRE)

/*
 * Definitions used for redirection table entry size.
 * Some RSS RETA sizes may not be supported by some drivers, check the
 * documentation or the description of relevant functions for more details.
 */
#define ETH_RSS_RETA_SIZE_64  64
#define ETH_RSS_RETA_SIZE_128 128
#define ETH_RSS_RETA_SIZE_256 256
#define ETH_RSS_RETA_SIZE_512 512
#define RTE_RETA_GROUP_SIZE   64

/* Definitions used for VMDQ and DCB functionality */
#define ETH_VMDQ_MAX_VLAN_FILTERS   64 /**< Maximum nb. of VMDQ vlan filters. */
#define ETH_DCB_NUM_USER_PRIORITIES 8  /**< Maximum nb. of DCB priorities. */
#define ETH_VMDQ_DCB_NUM_QUEUES     128 /**< Maximum nb. of VMDQ DCB queues. */
#define ETH_DCB_NUM_QUEUES          128 /**< Maximum nb. of DCB queues. */

/* DCB capability defines */
#define ETH_DCB_PG_SUPPORT      0x00000001 /**< Priority Group(ETS) support. */
#define ETH_DCB_PFC_SUPPORT     0x00000002 /**< Priority Flow Control support. */

/* Definitions used for VLAN Offload functionality */
#define ETH_VLAN_STRIP_OFFLOAD   0x0001 /**< VLAN Strip  On/Off */
#define ETH_VLAN_FILTER_OFFLOAD  0x0002 /**< VLAN Filter On/Off */
#define ETH_VLAN_EXTEND_OFFLOAD  0x0004 /**< VLAN Extend On/Off */

/* Definitions used for mask VLAN setting */
#define ETH_VLAN_STRIP_MASK   0x0001 /**< VLAN Strip  setting mask */
#define ETH_VLAN_FILTER_MASK  0x0002 /**< VLAN Filter  setting mask*/
#define ETH_VLAN_EXTEND_MASK  0x0004 /**< VLAN Extend  setting mask*/
#define ETH_VLAN_ID_MAX       0x0FFF /**< VLAN ID is in lower 12 bits*/

/* Definitions used for receive MAC address   */
#define ETH_NUM_RECEIVE_MAC_ADDR  128 /**< Maximum nb. of receive mac addr. */

/* Definitions used for unicast hash  */
#define ETH_VMDQ_NUM_UC_HASH_ARRAY  128 /**< Maximum nb. of UC hash array. */

/* Definitions used for VMDQ pool rx mode setting */
#define ETH_VMDQ_ACCEPT_UNTAG   0x0001 /**< accept untagged packets. */
#define ETH_VMDQ_ACCEPT_HASH_MC 0x0002 /**< accept packets in multicast table . */
#define ETH_VMDQ_ACCEPT_HASH_UC 0x0004 /**< accept packets in unicast table. */
#define ETH_VMDQ_ACCEPT_BROADCAST   0x0008 /**< accept broadcast packets. */
#define ETH_VMDQ_ACCEPT_MULTICAST   0x0010 /**< multicast promiscuous. */

/** Maximum nb. of vlan per mirror rule */
#define ETH_MIRROR_MAX_VLANS       64

#define ETH_MIRROR_VIRTUAL_POOL_UP     0x01  /**< Virtual Pool uplink Mirroring. */
#define ETH_MIRROR_UPLINK_PORT         0x02  /**< Uplink Port Mirroring. */
#define ETH_MIRROR_DOWNLINK_PORT       0x04  /**< Downlink Port Mirroring. */
#define ETH_MIRROR_VLAN                0x08  /**< VLAN Mirroring. */
#define ETH_MIRROR_VIRTUAL_POOL_DOWN   0x10  /**< Virtual Pool downlink Mirroring. */

/**
 * A structure used to configure VLAN traffic mirror of an Ethernet port.
 */
struct rte_eth_vlan_mirror {
	uint64_t vlan_mask; /**< mask for valid VLAN ID. */
	/** VLAN ID list for vlan mirroring. */
	uint16_t vlan_id[ETH_MIRROR_MAX_VLANS];
};

/**
 * A structure used to configure traffic mirror of an Ethernet port.
 */
struct rte_eth_mirror_conf {
	uint8_t rule_type; /**< Mirroring rule type */
	uint8_t dst_pool;  /**< Destination pool for this mirror rule. */
	uint64_t pool_mask; /**< Bitmap of pool for pool mirroring */
	/** VLAN ID setting for VLAN mirroring. */
	struct rte_eth_vlan_mirror vlan;
};

/**
 * A structure used to configure 64 entries of Redirection Table of the
 * Receive Side Scaling (RSS) feature of an Ethernet port. To configure
 * more than 64 entries supported by hardware, an array of this structure
 * is needed.
 */
struct rte_eth_rss_reta_entry64 {
	uint64_t mask;
	/**< Mask bits indicate which entries need to be updated/queried. */
	uint16_t reta[RTE_RETA_GROUP_SIZE];
	/**< Group of 64 redirection table entries. */
};

/**
 * This enum indicates the possible number of traffic classes
 * in DCB configurations
 */
enum rte_eth_nb_tcs {
	ETH_4_TCS = 4, /**< 4 TCs with DCB. */
	ETH_8_TCS = 8  /**< 8 TCs with DCB. */
};

/**
 * This enum indicates the possible number of queue pools
 * in VMDQ configurations.
 */
enum rte_eth_nb_pools {
	ETH_8_POOLS = 8,    /**< 8 VMDq pools. */
	ETH_16_POOLS = 16,  /**< 16 VMDq pools. */
	ETH_32_POOLS = 32,  /**< 32 VMDq pools. */
	ETH_64_POOLS = 64   /**< 64 VMDq pools. */
};

/* This structure may be extended in future. */
struct rte_eth_dcb_rx_conf {
	enum rte_eth_nb_tcs nb_tcs; /**< Possible DCB TCs, 4 or 8 TCs */
	/** Traffic class each UP mapped to. */
	uint8_t dcb_tc[ETH_DCB_NUM_USER_PRIORITIES];
};

struct rte_eth_vmdq_dcb_tx_conf {
	enum rte_eth_nb_pools nb_queue_pools; /**< With DCB, 16 or 32 pools. */
	/** Traffic class each UP mapped to. */
	uint8_t dcb_tc[ETH_DCB_NUM_USER_PRIORITIES];
};

struct rte_eth_dcb_tx_conf {
	enum rte_eth_nb_tcs nb_tcs; /**< Possible DCB TCs, 4 or 8 TCs. */
	/** Traffic class each UP mapped to. */
	uint8_t dcb_tc[ETH_DCB_NUM_USER_PRIORITIES];
};

struct rte_eth_vmdq_tx_conf {
	enum rte_eth_nb_pools nb_queue_pools; /**< VMDq mode, 64 pools. */
};

/**
 * A structure used to configure the VMDQ+DCB feature
 * of an Ethernet port.
 *
 * Using this feature, packets are routed to a pool of queues, based
 * on the vlan id in the vlan tag, and then to a specific queue within
 * that pool, using the user priority vlan tag field.
 *
 * A default pool may be used, if desired, to route all traffic which
 * does not match the vlan filter rules.
 */
struct rte_eth_vmdq_dcb_conf {
	enum rte_eth_nb_pools nb_queue_pools; /**< With DCB, 16 or 32 pools */
	uint8_t enable_default_pool; /**< If non-zero, use a default pool */
	uint8_t default_pool; /**< The default pool, if applicable */
	uint8_t nb_pool_maps; /**< We can have up to 64 filters/mappings */
	struct {
		uint16_t vlan_id; /**< The vlan id of the received frame */
		uint64_t pools;   /**< Bitmask of pools for packet rx */
	} pool_map[ETH_VMDQ_MAX_VLAN_FILTERS]; /**< VMDq vlan pool maps. */
	uint8_t dcb_tc[ETH_DCB_NUM_USER_PRIORITIES];
	/**< Selects a queue in a pool */
};

/**
 * A structure used to configure the VMDQ feature of an Ethernet port when
 * not combined with the DCB feature.
 *
 * Using this feature, packets are routed to a pool of queues. By default,
 * the pool selection is based on the MAC address, the vlan id in the
 * vlan tag as specified in the pool_map array.
 * Passing the ETH_VMDQ_ACCEPT_UNTAG in the rx_mode field allows pool
 * selection using only the MAC address. MAC address to pool mapping is done
 * using the rte_eth_dev_mac_addr_add function, with the pool parameter
 * corresponding to the pool id.
 *
 * Queue selection within the selected pool will be done using RSS when
 * it is enabled or revert to the first queue of the pool if not.
 *
 * A default pool may be used, if desired, to route all traffic which
 * does not match the vlan filter rules or any pool MAC address.
 */
struct rte_eth_vmdq_rx_conf {
	enum rte_eth_nb_pools nb_queue_pools; /**< VMDq only mode, 8 or 64 pools */
	uint8_t enable_default_pool; /**< If non-zero, use a default pool */
	uint8_t default_pool; /**< The default pool, if applicable */
	uint8_t enable_loop_back; /**< Enable VT loop back */
	uint8_t nb_pool_maps; /**< We can have up to 64 filters/mappings */
	uint32_t rx_mode; /**< Flags from ETH_VMDQ_ACCEPT_* */
	struct {
		uint16_t vlan_id; /**< The vlan id of the received frame */
		uint64_t pools;   /**< Bitmask of pools for packet rx */
	} pool_map[ETH_VMDQ_MAX_VLAN_FILTERS]; /**< VMDq vlan pool maps. */
};

/**
 * A structure used to configure the TX features of an Ethernet port.
 */
struct rte_eth_txmode {
	enum rte_eth_tx_mq_mode mq_mode; /**< TX multi-queues mode. */
	/**
	 * Per-port Tx offloads to be set using DEV_TX_OFFLOAD_* flags.
	 * Only offloads set on tx_offload_capa field on rte_eth_dev_info
	 * structure are allowed to be set.
	 */
	uint64_t offloads;

	/* For i40e specifically */
	uint16_t pvid;
	__extension__
	uint8_t hw_vlan_reject_tagged : 1,
		/**< If set, reject sending out tagged pkts */
		hw_vlan_reject_untagged : 1,
		/**< If set, reject sending out untagged pkts */
		hw_vlan_insert_pvid : 1;
		/**< If set, enable port based VLAN insertion */
};

/**
 * A structure used to configure an RX ring of an Ethernet port.
 */
struct rte_eth_rxconf {
	struct rte_eth_thresh rx_thresh; /**< RX ring threshold registers. */
	uint16_t rx_free_thresh; /**< Drives the freeing of RX descriptors. */
	uint8_t rx_drop_en; /**< Drop packets if no descriptors are available. */
	uint8_t rx_deferred_start; /**< Do not start queue with rte_eth_dev_start(). */
	/**
	 * Per-queue Rx offloads to be set using DEV_RX_OFFLOAD_* flags.
	 * Only offloads set on rx_queue_offload_capa or rx_offload_capa
	 * fields on rte_eth_dev_info structure are allowed to be set.
	 */
	uint64_t offloads;
};

/**
 * A structure used to configure a TX ring of an Ethernet port.
 */
struct rte_eth_txconf {
	struct rte_eth_thresh tx_thresh; /**< TX ring threshold registers. */
	uint16_t tx_rs_thresh; /**< Drives the setting of RS bit on TXDs. */
	uint16_t tx_free_thresh; /**< Start freeing TX buffers if there are
				      less free descriptors than this value. */

	uint8_t tx_deferred_start; /**< Do not start queue with rte_eth_dev_start(). */
	/**
	 * Per-queue Tx offloads to be set  using DEV_TX_OFFLOAD_* flags.
	 * Only offloads set on tx_queue_offload_capa or tx_offload_capa
	 * fields on rte_eth_dev_info structure are allowed to be set.
	 */
	uint64_t offloads;
};

/**
 * A structure contains information about HW descriptor ring limitations.
 */
struct rte_eth_desc_lim {
	uint16_t nb_max;   /**< Max allowed number of descriptors. */
	uint16_t nb_min;   /**< Min allowed number of descriptors. */
	uint16_t nb_align; /**< Number of descriptors should be aligned to. */

	/**
	 * Max allowed number of segments per whole packet.
	 *
	 * - For TSO packet this is the total number of data descriptors allowed
	 *   by device.
	 *
	 * @see nb_mtu_seg_max
	 */
	uint16_t nb_seg_max;

	/**
	 * Max number of segments per one MTU.
	 *
	 * - For non-TSO packet, this is the maximum allowed number of segments
	 *   in a single transmit packet.
	 *
	 * - For TSO packet each segment within the TSO may span up to this
	 *   value.
	 *
	 * @see nb_seg_max
	 */
	uint16_t nb_mtu_seg_max;
};

/**
 * This enum indicates the flow control mode
 */
enum rte_eth_fc_mode {
	RTE_FC_NONE = 0, /**< Disable flow control. */
	RTE_FC_RX_PAUSE, /**< RX pause frame, enable flowctrl on TX side. */
	RTE_FC_TX_PAUSE, /**< TX pause frame, enable flowctrl on RX side. */
	RTE_FC_FULL      /**< Enable flow control on both side. */
};

/**
 * A structure used to configure Ethernet flow control parameter.
 * These parameters will be configured into the register of the NIC.
 * Please refer to the corresponding data sheet for proper value.
 */
struct rte_eth_fc_conf {
	uint32_t high_water;  /**< High threshold value to trigger XOFF */
	uint32_t low_water;   /**< Low threshold value to trigger XON */
	uint16_t pause_time;  /**< Pause quota in the Pause frame */
	uint16_t send_xon;    /**< Is XON frame need be sent */
	enum rte_eth_fc_mode mode;  /**< Link flow control mode */
	uint8_t mac_ctrl_frame_fwd; /**< Forward MAC control frames */
	uint8_t autoneg;      /**< Use Pause autoneg */
};

/**
 * A structure used to configure Ethernet priority flow control parameter.
 * These parameters will be configured into the register of the NIC.
 * Please refer to the corresponding data sheet for proper value.
 */
struct rte_eth_pfc_conf {
	struct rte_eth_fc_conf fc; /**< General flow control parameter. */
	uint8_t priority;          /**< VLAN User Priority. */
};

/**
 *  Memory space that can be configured to store Flow Director filters
 *  in the board memory.
 */
enum rte_fdir_pballoc_type {
	RTE_FDIR_PBALLOC_64K = 0,  /**< 64k. */
	RTE_FDIR_PBALLOC_128K,     /**< 128k. */
	RTE_FDIR_PBALLOC_256K,     /**< 256k. */
};

/**
 *  Select report mode of FDIR hash information in RX descriptors.
 */
enum rte_fdir_status_mode {
	RTE_FDIR_NO_REPORT_STATUS = 0, /**< Never report FDIR hash. */
	RTE_FDIR_REPORT_STATUS, /**< Only report FDIR hash for matching pkts. */
	RTE_FDIR_REPORT_STATUS_ALWAYS, /**< Always report FDIR hash. */
};

/**
 * A structure used to configure the Flow Director (FDIR) feature
 * of an Ethernet port.
 *
 * If mode is RTE_FDIR_DISABLE, the pballoc value is ignored.
 */
struct rte_fdir_conf {
	enum rte_fdir_mode mode; /**< Flow Director mode. */
	enum rte_fdir_pballoc_type pballoc; /**< Space for FDIR filters. */
	enum rte_fdir_status_mode status;  /**< How to report FDIR hash. */
	/** RX queue of packets matching a "drop" filter in perfect mode. */
	uint8_t drop_queue;
	struct rte_eth_fdir_masks mask;
	struct rte_eth_fdir_flex_conf flex_conf;
	/**< Flex payload configuration. */
};

/**
 * UDP tunneling configuration.
 * Used to config the UDP port for a type of tunnel.
 * NICs need the UDP port to identify the tunnel type.
 * Normally a type of tunnel has a default UDP port, this structure can be used
 * in case if the users want to change or support more UDP port.
 */
struct rte_eth_udp_tunnel {
	uint16_t udp_port; /**< UDP port used for the tunnel. */
	uint8_t prot_type; /**< Tunnel type. Defined in rte_eth_tunnel_type. */
};

/**
 * A structure used to enable/disable specific device interrupts.
 */
struct rte_intr_conf {
	/** enable/disable lsc interrupt. 0 (default) - disable, 1 enable */
	uint32_t lsc:1;
	/** enable/disable rxq interrupt. 0 (default) - disable, 1 enable */
	uint32_t rxq:1;
	/** enable/disable rmv interrupt. 0 (default) - disable, 1 enable */
	uint32_t rmv:1;
};

/**
 * A structure used to configure an Ethernet port.
 * Depending upon the RX multi-queue mode, extra advanced
 * configuration settings may be needed.
 */
struct rte_eth_conf {
	uint32_t link_speeds; /**< bitmap of ETH_LINK_SPEED_XXX of speeds to be
				used. ETH_LINK_SPEED_FIXED disables link
				autonegotiation, and a unique speed shall be
				set. Otherwise, the bitmap defines the set of
				speeds to be advertised. If the special value
				ETH_LINK_SPEED_AUTONEG (0) is used, all speeds
				supported are advertised. */
	struct rte_eth_rxmode rxmode; /**< Port RX configuration. */
	struct rte_eth_txmode txmode; /**< Port TX configuration. */
	uint32_t lpbk_mode; /**< Loopback operation mode. By default the value
			         is 0, meaning the loopback mode is disabled.
				 Read the datasheet of given ethernet controller
				 for details. The possible values of this field
				 are defined in implementation of each driver. */
	struct {
		struct rte_eth_rss_conf rss_conf; /**< Port RSS configuration */
		struct rte_eth_vmdq_dcb_conf vmdq_dcb_conf;
		/**< Port vmdq+dcb configuration. */
		struct rte_eth_dcb_rx_conf dcb_rx_conf;
		/**< Port dcb RX configuration. */
		struct rte_eth_vmdq_rx_conf vmdq_rx_conf;
		/**< Port vmdq RX configuration. */
	} rx_adv_conf; /**< Port RX filtering configuration. */
	union {
		struct rte_eth_vmdq_dcb_tx_conf vmdq_dcb_tx_conf;
		/**< Port vmdq+dcb TX configuration. */
		struct rte_eth_dcb_tx_conf dcb_tx_conf;
		/**< Port dcb TX configuration. */
		struct rte_eth_vmdq_tx_conf vmdq_tx_conf;
		/**< Port vmdq TX configuration. */
	} tx_adv_conf; /**< Port TX DCB configuration (union). */
	/** Currently,Priority Flow Control(PFC) are supported,if DCB with PFC
	    is needed,and the variable must be set ETH_DCB_PFC_SUPPORT. */
	uint32_t dcb_capability_en;
	struct rte_fdir_conf fdir_conf; /**< FDIR configuration. */
	struct rte_intr_conf intr_conf; /**< Interrupt mode configuration. */
};

/**
 * RX offload capabilities of a device.
 */
#define DEV_RX_OFFLOAD_VLAN_STRIP  0x00000001
#define DEV_RX_OFFLOAD_IPV4_CKSUM  0x00000002
#define DEV_RX_OFFLOAD_UDP_CKSUM   0x00000004
#define DEV_RX_OFFLOAD_TCP_CKSUM   0x00000008
#define DEV_RX_OFFLOAD_TCP_LRO     0x00000010
#define DEV_RX_OFFLOAD_QINQ_STRIP  0x00000020
#define DEV_RX_OFFLOAD_OUTER_IPV4_CKSUM 0x00000040
#define DEV_RX_OFFLOAD_MACSEC_STRIP     0x00000080
#define DEV_RX_OFFLOAD_HEADER_SPLIT	0x00000100
#define DEV_RX_OFFLOAD_VLAN_FILTER	0x00000200
#define DEV_RX_OFFLOAD_VLAN_EXTEND	0x00000400
#define DEV_RX_OFFLOAD_JUMBO_FRAME	0x00000800
#define DEV_RX_OFFLOAD_SCATTER		0x00002000
#define DEV_RX_OFFLOAD_TIMESTAMP	0x00004000
#define DEV_RX_OFFLOAD_SECURITY         0x00008000
#define DEV_RX_OFFLOAD_KEEP_CRC		0x00010000
#define DEV_RX_OFFLOAD_SCTP_CKSUM	0x00020000
#define DEV_RX_OFFLOAD_OUTER_UDP_CKSUM  0x00040000

#define DEV_RX_OFFLOAD_CHECKSUM (DEV_RX_OFFLOAD_IPV4_CKSUM | \
				 DEV_RX_OFFLOAD_UDP_CKSUM | \
				 DEV_RX_OFFLOAD_TCP_CKSUM)
#define DEV_RX_OFFLOAD_VLAN (DEV_RX_OFFLOAD_VLAN_STRIP | \
			     DEV_RX_OFFLOAD_VLAN_FILTER | \
			     DEV_RX_OFFLOAD_VLAN_EXTEND)

/*
 * If new Rx offload capabilities are defined, they also must be
 * mentioned in rte_rx_offload_names in rte_ethdev.c file.
 */

/**
 * TX offload capabilities of a device.
 */
#define DEV_TX_OFFLOAD_VLAN_INSERT 0x00000001
#define DEV_TX_OFFLOAD_IPV4_CKSUM  0x00000002
#define DEV_TX_OFFLOAD_UDP_CKSUM   0x00000004
#define DEV_TX_OFFLOAD_TCP_CKSUM   0x00000008
#define DEV_TX_OFFLOAD_SCTP_CKSUM  0x00000010
#define DEV_TX_OFFLOAD_TCP_TSO     0x00000020
#define DEV_TX_OFFLOAD_UDP_TSO     0x00000040
#define DEV_TX_OFFLOAD_OUTER_IPV4_CKSUM 0x00000080 /**< Used for tunneling packet. */
#define DEV_TX_OFFLOAD_QINQ_INSERT 0x00000100
#define DEV_TX_OFFLOAD_VXLAN_TNL_TSO    0x00000200    /**< Used for tunneling packet. */
#define DEV_TX_OFFLOAD_GRE_TNL_TSO      0x00000400    /**< Used for tunneling packet. */
#define DEV_TX_OFFLOAD_IPIP_TNL_TSO     0x00000800    /**< Used for tunneling packet. */
#define DEV_TX_OFFLOAD_GENEVE_TNL_TSO   0x00001000    /**< Used for tunneling packet. */
#define DEV_TX_OFFLOAD_MACSEC_INSERT    0x00002000
#define DEV_TX_OFFLOAD_MT_LOCKFREE      0x00004000
/**< Multiple threads can invoke rte_eth_tx_burst() concurrently on the same
 * tx queue without SW lock.
 */
#define DEV_TX_OFFLOAD_MULTI_SEGS	0x00008000
/**< Device supports multi segment send. */
#define DEV_TX_OFFLOAD_MBUF_FAST_FREE	0x00010000
/**< Device supports optimization for fast release of mbufs.
 *   When set application must guarantee that per-queue all mbufs comes from
 *   the same mempool and has refcnt = 1.
 */
#define DEV_TX_OFFLOAD_SECURITY         0x00020000
/**
 * Device supports generic UDP tunneled packet TSO.
 * Application must set PKT_TX_TUNNEL_UDP and other mbuf fields required
 * for tunnel TSO.
 */
#define DEV_TX_OFFLOAD_UDP_TNL_TSO      0x00040000
/**
 * Device supports generic IP tunneled packet TSO.
 * Application must set PKT_TX_TUNNEL_IP and other mbuf fields required
 * for tunnel TSO.
 */
#define DEV_TX_OFFLOAD_IP_TNL_TSO       0x00080000
/** Device supports outer UDP checksum */
#define DEV_TX_OFFLOAD_OUTER_UDP_CKSUM  0x00100000
/**
 * Device supports match on metadata Tx offload..
 * Application must set PKT_TX_METADATA and mbuf metadata field.
 */
#define DEV_TX_OFFLOAD_MATCH_METADATA   0x00200000

#define RTE_ETH_DEV_CAPA_RUNTIME_RX_QUEUE_SETUP 0x00000001
/**< Device supports Rx queue setup after device started*/
#define RTE_ETH_DEV_CAPA_RUNTIME_TX_QUEUE_SETUP 0x00000002
/**< Device supports Tx queue setup after device started*/

/*
 * If new Tx offload capabilities are defined, they also must be
 * mentioned in rte_tx_offload_names in rte_ethdev.c file.
 */

/*
 * Fallback default preferred Rx/Tx port parameters.
 * These are used if an application requests default parameters
 * but the PMD does not provide preferred values.
 */
#define RTE_ETH_DEV_FALLBACK_RX_RINGSIZE 512
#define RTE_ETH_DEV_FALLBACK_TX_RINGSIZE 512
#define RTE_ETH_DEV_FALLBACK_RX_NBQUEUES 1
#define RTE_ETH_DEV_FALLBACK_TX_NBQUEUES 1

/**
 * Preferred Rx/Tx port parameters.
 * There are separate instances of this structure for transmission
 * and reception respectively.
 */
struct rte_eth_dev_portconf {
	uint16_t burst_size; /**< Device-preferred burst size */
	uint16_t ring_size; /**< Device-preferred size of queue rings */
	uint16_t nb_queues; /**< Device-preferred number of queues */
};

/**
 * Default values for switch domain id when ethdev does not support switch
 * domain definitions.
 */
#define RTE_ETH_DEV_SWITCH_DOMAIN_ID_INVALID	(0)

/**
 * Ethernet device associated switch information
 */
struct rte_eth_switch_info {
	const char *name;	/**< switch name */
	uint16_t domain_id;	/**< switch domain id */
	uint16_t port_id;
	/**<
	 * mapping to the devices physical switch port as enumerated from the
	 * perspective of the embedded interconnect/switch. For SR-IOV enabled
	 * device this may correspond to the VF_ID of each virtual function,
	 * but each driver should explicitly define the mapping of switch
	 * port identifier to that physical interconnect/switch
	 */
};

/**
 * Ethernet device information
 */

/**
 * A structure used to retrieve the contextual information of
 * an Ethernet device, such as the controlling driver of the
 * device, etc...
 */
struct rte_eth_dev_info {
	struct rte_device *device; /** Generic device information */
	const char *driver_name; /**< Device Driver name. */
	unsigned int if_index; /**< Index to bound host interface, or 0 if none.
		Use if_indextoname() to translate into an interface name. */
	const uint32_t *dev_flags; /**< Device flags */
	uint32_t min_rx_bufsize; /**< Minimum size of RX buffer. */
	uint32_t max_rx_pktlen; /**< Maximum configurable length of RX pkt. */
	uint16_t max_rx_queues; /**< Maximum number of RX queues. */
	uint16_t max_tx_queues; /**< Maximum number of TX queues. */
	uint32_t max_mac_addrs; /**< Maximum number of MAC addresses. */
	uint32_t max_hash_mac_addrs;
	/** Maximum number of hash MAC addresses for MTA and UTA. */
	uint16_t max_vfs; /**< Maximum number of VFs. */
	uint16_t max_vmdq_pools; /**< Maximum number of VMDq pools. */
	uint64_t rx_offload_capa;
	/**< All RX offload capabilities including all per-queue ones */
	uint64_t tx_offload_capa;
	/**< All TX offload capabilities including all per-queue ones */
	uint64_t rx_queue_offload_capa;
	/**< Device per-queue RX offload capabilities. */
	uint64_t tx_queue_offload_capa;
	/**< Device per-queue TX offload capabilities. */
	uint16_t reta_size;
	/**< Device redirection table size, the total number of entries. */
	uint8_t hash_key_size; /**< Hash key size in bytes */
	/** Bit mask of RSS offloads, the bit offset also means flow type */
	uint64_t flow_type_rss_offloads;
	struct rte_eth_rxconf default_rxconf; /**< Default RX configuration */
	struct rte_eth_txconf default_txconf; /**< Default TX configuration */
	uint16_t vmdq_queue_base; /**< First queue ID for VMDQ pools. */
	uint16_t vmdq_queue_num;  /**< Queue number for VMDQ pools. */
	uint16_t vmdq_pool_base;  /**< First ID of VMDQ pools. */
	struct rte_eth_desc_lim rx_desc_lim;  /**< RX descriptors limits */
	struct rte_eth_desc_lim tx_desc_lim;  /**< TX descriptors limits */
	uint32_t speed_capa;  /**< Supported speeds bitmap (ETH_LINK_SPEED_). */
	/** Configured number of rx/tx queues */
	uint16_t nb_rx_queues; /**< Number of RX queues. */
	uint16_t nb_tx_queues; /**< Number of TX queues. */
	/** Rx parameter recommendations */
	struct rte_eth_dev_portconf default_rxportconf;
	/** Tx parameter recommendations */
	struct rte_eth_dev_portconf default_txportconf;
	/** Generic device capabilities (RTE_ETH_DEV_CAPA_). */
	uint64_t dev_capa;
	/**
	 * Switching information for ports on a device with a
	 * embedded managed interconnect/switch.
	 */
	struct rte_eth_switch_info switch_info;
};

/**
 * Ethernet device RX queue information structure.
 * Used to retieve information about configured queue.
 */
struct rte_eth_rxq_info {
	struct rte_mempool *mp;     /**< mempool used by that queue. */
	struct rte_eth_rxconf conf; /**< queue config parameters. */
	uint8_t scattered_rx;       /**< scattered packets RX supported. */
	uint16_t nb_desc;           /**< configured number of RXDs. */
} ;

/**
 * Ethernet device TX queue information structure.
 * Used to retrieve information about configured queue.
 */
struct rte_eth_txq_info {
	struct rte_eth_txconf conf; /**< queue config parameters. */
	uint16_t nb_desc;           /**< configured number of TXDs. */
} ;

/** Maximum name length for extended statistics counters */
#define RTE_ETH_XSTATS_NAME_SIZE 64

/**
 * An Ethernet device extended statistic structure
 *
 * This structure is used by rte_eth_xstats_get() to provide
 * statistics that are not provided in the generic *rte_eth_stats*
 * structure.
 * It maps a name id, corresponding to an index in the array returned
 * by rte_eth_xstats_get_names(), to a statistic value.
 */
struct rte_eth_xstat {
	uint64_t id;        /**< The index in xstats name array. */
	uint64_t value;     /**< The statistic counter value. */
};

/**
 * A name element for extended statistics.
 *
 * An array of this structure is returned by rte_eth_xstats_get_names().
 * It lists the names of extended statistics for a PMD. The *rte_eth_xstat*
 * structure references these names by their array index.
 */
struct rte_eth_xstat_name {
	char name[RTE_ETH_XSTATS_NAME_SIZE]; /**< The statistic name. */
};

#define ETH_DCB_NUM_TCS    8
#define ETH_MAX_VMDQ_POOL  64

/**
 * A structure used to get the information of queue and
 * TC mapping on both TX and RX paths.
 */
struct rte_eth_dcb_tc_queue_mapping {
	/** rx queues assigned to tc per Pool */
	struct {
		uint8_t base;
		uint8_t nb_queue;
	} tc_rxq[ETH_MAX_VMDQ_POOL][ETH_DCB_NUM_TCS];
	/** rx queues assigned to tc per Pool */
	struct {
		uint8_t base;
		uint8_t nb_queue;
	} tc_txq[ETH_MAX_VMDQ_POOL][ETH_DCB_NUM_TCS];
};

/**
 * A structure used to get the information of DCB.
 * It includes TC UP mapping and queue TC mapping.
 */
struct rte_eth_dcb_info {
	uint8_t nb_tcs;        /**< number of TCs */
	uint8_t prio_tc[ETH_DCB_NUM_USER_PRIORITIES]; /**< Priority to tc */
	uint8_t tc_bws[ETH_DCB_NUM_TCS]; /**< TX BW percentage for each TC */
	/** rx queues assigned to tc */
	struct rte_eth_dcb_tc_queue_mapping tc_queue;
};

/**
 * RX/TX queue states
 */
#define RTE_ETH_QUEUE_STATE_STOPPED 0
#define RTE_ETH_QUEUE_STATE_STARTED 1

#define RTE_ETH_ALL RTE_MAX_ETHPORTS

/* Macros to check for valid port */
#define RTE_ETH_VALID_PORTID_OR_ERR_RET(port_id, retval) do { \
	if (!rte_eth_dev_is_valid_port(port_id)) { \
		RTE_ETHDEV_LOG(ERR, "Invalid port_id=%u\n", port_id); \
		return retval; \
	} \
} while (0)

#define RTE_ETH_VALID_PORTID_OR_RET(port_id) do { \
	if (!rte_eth_dev_is_valid_port(port_id)) { \
		RTE_ETHDEV_LOG(ERR, "Invalid port_id=%u\n", port_id); \
		return; \
	} \
} while (0)

/**
 * l2 tunnel configuration.
 */

/**< l2 tunnel enable mask */
#define ETH_L2_TUNNEL_ENABLE_MASK       0x00000001
/**< l2 tunnel insertion mask */
#define ETH_L2_TUNNEL_INSERTION_MASK    0x00000002
/**< l2 tunnel stripping mask */
#define ETH_L2_TUNNEL_STRIPPING_MASK    0x00000004
/**< l2 tunnel forwarding mask */
#define ETH_L2_TUNNEL_FORWARDING_MASK   0x00000008

/**
 * Function type used for RX packet processing packet callbacks.
 *
 * The callback function is called on RX with a burst of packets that have
 * been received on the given port and queue.
 *
 * @param port_id
 *   The Ethernet port on which RX is being performed.
 * @param queue
 *   The queue on the Ethernet port which is being used to receive the packets.
 * @param pkts
 *   The burst of packets that have just been received.
 * @param nb_pkts
 *   The number of packets in the burst pointed to by "pkts".
 * @param max_pkts
 *   The max number of packets that can be stored in the "pkts" array.
 * @param user_param
 *   The arbitrary user parameter passed in by the application when the callback
 *   was originally configured.
 * @return
 *   The number of packets returned to the user.
 */
typedef uint16_t (*rte_rx_callback_fn)(uint16_t port_id, uint16_t queue,
	struct rte_mbuf *pkts[], uint16_t nb_pkts, uint16_t max_pkts,
	void *user_param);

/**
 * Function type used for TX packet processing packet callbacks.
 *
 * The callback function is called on TX with a burst of packets immediately
 * before the packets are put onto the hardware queue for transmission.
 *
 * @param port_id
 *   The Ethernet port on which TX is being performed.
 * @param queue
 *   The queue on the Ethernet port which is being used to transmit the packets.
 * @param pkts
 *   The burst of packets that are about to be transmitted.
 * @param nb_pkts
 *   The number of packets in the burst pointed to by "pkts".
 * @param user_param
 *   The arbitrary user parameter passed in by the application when the callback
 *   was originally configured.
 * @return
 *   The number of packets to be written to the NIC.
 */
typedef uint16_t (*rte_tx_callback_fn)(uint16_t port_id, uint16_t queue,
	struct rte_mbuf *pkts[], uint16_t nb_pkts, void *user_param);

/**
 * Possible states of an ethdev port.
 */
enum rte_eth_dev_state {
	/** Device is unused before being probed. */
	RTE_ETH_DEV_UNUSED = 0,
	/** Device is attached when allocated in probing. */
	RTE_ETH_DEV_ATTACHED,
	/** Device is in removed state when plug-out is detected. */
	RTE_ETH_DEV_REMOVED,
};

struct rte_eth_dev_sriov {
	uint8_t active;               /**< SRIOV is active with 16, 32 or 64 pools */
	uint8_t nb_q_per_pool;        /**< rx queue number per pool */
	uint16_t def_vmdq_idx;        /**< Default pool num used for PF */
	uint16_t def_pool_q_idx;      /**< Default pool queue start reg index */
};
#define RTE_ETH_DEV_SRIOV(dev)         ((dev)->data->sriov)

#define RTE_ETH_NAME_MAX_LEN RTE_DEV_NAME_MAX_LEN

#define RTE_ETH_DEV_NO_OWNER 0

#define RTE_ETH_MAX_OWNER_NAME_LEN 64

struct rte_eth_dev_owner {
	uint64_t id; /**< The owner unique identifier. */
	char name[RTE_ETH_MAX_OWNER_NAME_LEN]; /**< The owner name. */
};

/**
 * Port is released (i.e. totally freed and data erased) on close.
 * Temporary flag for PMD migration to new rte_eth_dev_close() behaviour.
 */
#define RTE_ETH_DEV_CLOSE_REMOVE 0x0001
/** Device supports link state interrupt */
#define RTE_ETH_DEV_INTR_LSC     0x0002
/** Device is a bonded slave */
#define RTE_ETH_DEV_BONDED_SLAVE 0x0004
/** Device supports device removal interrupt */
#define RTE_ETH_DEV_INTR_RMV     0x0008
/** Device is port representor */
#define RTE_ETH_DEV_REPRESENTOR  0x0010
/** Device does not support MAC change after started */
#define RTE_ETH_DEV_NOLIVE_MAC_ADDR  0x0020

/**
 * Iterates over valid ethdev ports owned by a specific owner.
 *
 * @param port_id
 *   The id of the next possible valid owned port.
 * @param	owner_id
 *  The owner identifier.
 *  RTE_ETH_DEV_NO_OWNER means iterate over all valid ownerless ports.
 * @return
 *   Next valid port id owned by owner_id, RTE_MAX_ETHPORTS if there is none.
 */
uint64_t rte_eth_find_next_owned_by(uint16_t port_id,
		const uint64_t owner_id);

/**
 * Macro to iterate over all enabled ethdev ports owned by a specific owner.
 */
#define RTE_ETH_FOREACH_DEV_OWNED_BY(p, o) \
	for (p = rte_eth_find_next_owned_by(0, o); \
	     (unsigned int)p < (unsigned int)RTE_MAX_ETHPORTS; \
	     p = rte_eth_find_next_owned_by(p + 1, o))

/**
 * Iterates over valid ethdev ports.
 *
 * @param port_id
 *   The id of the next possible valid port.
 * @return
 *   Next valid port id, RTE_MAX_ETHPORTS if there is none.
 */
uint16_t rte_eth_find_next(uint16_t port_id);

/**
 * Macro to iterate over all enabled and ownerless ethdev ports.
 */
#define RTE_ETH_FOREACH_DEV(p) \
	RTE_ETH_FOREACH_DEV_OWNED_BY(p, RTE_ETH_DEV_NO_OWNER)

#define __rte_experimental

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice.
 *
 * Get a new unique owner identifier.
 * An owner identifier is used to owns Ethernet devices by only one DPDK entity
 * to avoid multiple management of device by different entities.
 *
 * @param	owner_id
 *   Owner identifier pointer.
 * @return
 *   Negative errno value on error, 0 on success.
 */
int __rte_experimental rte_eth_dev_owner_new(uint64_t *owner_id);

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice.
 *
 * Set an Ethernet device owner.
 *
 * @param	port_id
 *  The identifier of the port to own.
 * @param	owner
 *  The owner pointer.
 * @return
 *  Negative errno value on error, 0 on success.
 */
int __rte_experimental rte_eth_dev_owner_set(const uint16_t port_id,
                                             const struct rte_eth_dev_owner *owner);

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice.
 *
 * Unset Ethernet device owner to make the device ownerless.
 *
 * @param	port_id
 *  The identifier of port to make ownerless.
 * @param	owner_id
 *  The owner identifier.
 * @return
 *  0 on success, negative errno value on error.
 */
int __rte_experimental rte_eth_dev_owner_unset(const uint16_t port_id,
                                               const uint64_t owner_id);

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice.
 *
 * Remove owner from all Ethernet devices owned by a specific owner.
 *
 * @param	owner_id
 *  The owner identifier.
 */
void __rte_experimental rte_eth_dev_owner_delete(const uint64_t owner_id);

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice.
 *
 * Get the owner of an Ethernet device.
 *
 * @param	port_id
 *  The port identifier.
 * @param	owner
 *  The owner structure pointer to fill.
 * @return
 *  0 on success, negative errno value on error..
 */
int __rte_experimental rte_eth_dev_owner_get(const uint16_t port_id,
                                             struct rte_eth_dev_owner *owner);

/**
 * Get the total number of Ethernet devices that have been successfully
 * initialized by the matching Ethernet driver during the PCI probing phase
 * and that are available for applications to use. These devices must be
 * accessed by using the ``RTE_ETH_FOREACH_DEV()`` macro to deal with
 * non-contiguous ranges of devices.
 * These non-contiguous ranges can be created by calls to hotplug functions or
 * by some PMDs.
 *
 * @return
 *   - The total number of usable Ethernet devices.
 */
__rte_deprecated
uint16_t rte_eth_dev_count(void);

/**
 * Get the number of ports which are usable for the application.
 *
 * These devices must be iterated by using the macro
 * ``RTE_ETH_FOREACH_DEV`` or ``RTE_ETH_FOREACH_DEV_OWNED_BY``
 * to deal with non-contiguous ranges of devices.
 *
 * @return
 *   The count of available Ethernet devices.
 */
uint16_t rte_eth_dev_count_avail(void);

/**
 * Get the total number of ports which are allocated.
 *
 * Some devices may not be available for the application.
 *
 * @return
 *   The total count of Ethernet devices.
 */
uint16_t __rte_experimental rte_eth_dev_count_total(void);

/**
 * Convert a numerical speed in Mbps to a bitmap flag that can be used in
 * the bitmap link_speeds of the struct rte_eth_conf
 *
 * @param speed
 *   Numerical speed value in Mbps
 * @param duplex
 *   ETH_LINK_[HALF/FULL]_DUPLEX (only for 10/100M speeds)
 * @return
 *   0 if the speed cannot be mapped
 */
uint32_t rte_eth_speed_bitflag(uint32_t speed, int duplex);

/**
 * Get DEV_RX_OFFLOAD_* flag name.
 *
 * @param offload
 *   Offload flag.
 * @return
 *   Offload name or 'UNKNOWN' if the flag cannot be recognised.
 */
const char *rte_eth_dev_rx_offload_name(uint64_t offload);

/**
 * Get DEV_TX_OFFLOAD_* flag name.
 *
 * @param offload
 *   Offload flag.
 * @return
 *   Offload name or 'UNKNOWN' if the flag cannot be recognised.
 */
const char *rte_eth_dev_tx_offload_name(uint64_t offload);

/**
 * Configure an Ethernet device.
 * This function must be invoked first before any other function in the
 * Ethernet API. This function can also be re-invoked when a device is in the
 * stopped state.
 *
 * @param port_id
 *   The port identifier of the Ethernet device to configure.
 * @param nb_rx_queue
 *   The number of receive queues to set up for the Ethernet device.
 * @param nb_tx_queue
 *   The number of transmit queues to set up for the Ethernet device.
 * @param eth_conf
 *   The pointer to the configuration data to be used for the Ethernet device.
 *   The *rte_eth_conf* structure includes:
 *     -  the hardware offload features to activate, with dedicated fields for
 *        each statically configurable offload hardware feature provided by
 *        Ethernet devices, such as IP checksum or VLAN tag stripping for
 *        example.
 *        The Rx offload bitfield API is obsolete and will be deprecated.
 *        Applications should set the ignore_bitfield_offloads bit on *rxmode*
 *        structure and use offloads field to set per-port offloads instead.
 *     -  Any offloading set in eth_conf->[rt]xmode.offloads must be within
 *        the [rt]x_offload_capa returned from rte_eth_dev_infos_get().
 *        Any type of device supported offloading set in the input argument
 *        eth_conf->[rt]xmode.offloads to rte_eth_dev_configure() is enabled
 *        on all queues and it can't be disabled in rte_eth_[rt]x_queue_setup()
 *     -  the Receive Side Scaling (RSS) configuration when using multiple RX
 *        queues per port. Any RSS hash function set in eth_conf->rss_conf.rss_hf
 *        must be within the flow_type_rss_offloads provided by drivers via
 *        rte_eth_dev_infos_get() API.
 *
 *   Embedding all configuration information in a single data structure
 *   is the more flexible method that allows the addition of new features
 *   without changing the syntax of the API.
 * @return
 *   - 0: Success, device configured.
 *   - <0: Error code returned by the driver configuration function.
 */
int rte_eth_dev_configure(uint16_t port_id, uint16_t nb_rx_queue,
                          uint16_t nb_tx_queue, const struct rte_eth_conf *eth_conf);

/**
 * @warning
 * @b EXPERIMENTAL: this API may change without prior notice.
 *
 * Check if an Ethernet device was physically removed.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @return
 *   1 when the Ethernet device is removed, otherwise 0.
 */
int __rte_experimental
rte_eth_dev_is_removed(uint16_t port_id);

/**
 * Allocate and set up a receive queue for an Ethernet device.
 *
 * The function allocates a contiguous block of memory for *nb_rx_desc*
 * receive descriptors from a memory zone associated with *socket_id*
 * and initializes each receive descriptor with a network buffer allocated
 * from the memory pool *mb_pool*.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param rx_queue_id
 *   The index of the receive queue to set up.
 *   The value must be in the range [0, nb_rx_queue - 1] previously supplied
 *   to rte_eth_dev_configure().
 * @param nb_rx_desc
 *   The number of receive descriptors to allocate for the receive ring.
 * @param socket_id
 *   The *socket_id* argument is the socket identifier in case of NUMA.
 *   The value can be *SOCKET_ID_ANY* if there is no NUMA constraint for
 *   the DMA memory allocated for the receive descriptors of the ring.
 * @param rx_conf
 *   The pointer to the configuration data to be used for the receive queue.
 *   NULL value is allowed, in which case default RX configuration
 *   will be used.
 *   The *rx_conf* structure contains an *rx_thresh* structure with the values
 *   of the Prefetch, Host, and Write-Back threshold registers of the receive
 *   ring.
 *   In addition it contains the hardware offloads features to activate using
 *   the DEV_RX_OFFLOAD_* flags.
 *   If an offloading set in rx_conf->offloads
 *   hasn't been set in the input argument eth_conf->rxmode.offloads
 *   to rte_eth_dev_configure(), it is a new added offloading, it must be
 *   per-queue type and it is enabled for the queue.
 *   No need to repeat any bit in rx_conf->offloads which has already been
 *   enabled in rte_eth_dev_configure() at port level. An offloading enabled
 *   at port level can't be disabled at queue level.
 * @param mb_pool
 *   The pointer to the memory pool from which to allocate *rte_mbuf* network
 *   memory buffers to populate each descriptor of the receive ring.
 * @return
 *   - 0: Success, receive queue correctly set up.
 *   - -EIO: if device is removed.
 *   - -EINVAL: The size of network buffers which can be allocated from the
 *      memory pool does not fit the various buffer sizes allowed by the
 *      device controller.
 *   - -ENOMEM: Unable to allocate the receive ring descriptors or to
 *      allocate network memory buffers from the memory pool when
 *      initializing receive descriptors.
 */
int rte_eth_rx_queue_setup(uint16_t port_id, uint16_t rx_queue_id,
                           uint16_t nb_rx_desc, unsigned int socket_id,
                           const struct rte_eth_rxconf *rx_conf,
                           struct rte_mempool *mb_pool);

/**
 * Allocate and set up a transmit queue for an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param tx_queue_id
 *   The index of the transmit queue to set up.
 *   The value must be in the range [0, nb_tx_queue - 1] previously supplied
 *   to rte_eth_dev_configure().
 * @param nb_tx_desc
 *   The number of transmit descriptors to allocate for the transmit ring.
 * @param socket_id
 *   The *socket_id* argument is the socket identifier in case of NUMA.
 *   Its value can be *SOCKET_ID_ANY* if there is no NUMA constraint for
 *   the DMA memory allocated for the transmit descriptors of the ring.
 * @param tx_conf
 *   The pointer to the configuration data to be used for the transmit queue.
 *   NULL value is allowed, in which case default TX configuration
 *   will be used.
 *   The *tx_conf* structure contains the following data:
 *   - The *tx_thresh* structure with the values of the Prefetch, Host, and
 *     Write-Back threshold registers of the transmit ring.
 *     When setting Write-Back threshold to the value greater then zero,
 *     *tx_rs_thresh* value should be explicitly set to one.
 *   - The *tx_free_thresh* value indicates the [minimum] number of network
 *     buffers that must be pending in the transmit ring to trigger their
 *     [implicit] freeing by the driver transmit function.
 *   - The *tx_rs_thresh* value indicates the [minimum] number of transmit
 *     descriptors that must be pending in the transmit ring before setting the
 *     RS bit on a descriptor by the driver transmit function.
 *     The *tx_rs_thresh* value should be less or equal then
 *     *tx_free_thresh* value, and both of them should be less then
 *     *nb_tx_desc* - 3.
 *   - The *offloads* member contains Tx offloads to be enabled.
 *     If an offloading set in tx_conf->offloads
 *     hasn't been set in the input argument eth_conf->txmode.offloads
 *     to rte_eth_dev_configure(), it is a new added offloading, it must be
 *     per-queue type and it is enabled for the queue.
 *     No need to repeat any bit in tx_conf->offloads which has already been
 *     enabled in rte_eth_dev_configure() at port level. An offloading enabled
 *     at port level can't be disabled at queue level.
 *
 *     Note that setting *tx_free_thresh* or *tx_rs_thresh* value to 0 forces
 *     the transmit function to use default values.
 * @return
 *   - 0: Success, the transmit queue is correctly set up.
 *   - -ENOMEM: Unable to allocate the transmit ring descriptors.
 */
int rte_eth_tx_queue_setup(uint16_t port_id, uint16_t tx_queue_id,
                           uint16_t nb_tx_desc, unsigned int socket_id,
                           const struct rte_eth_txconf *tx_conf);

/**
 * Return the NUMA socket to which an Ethernet device is connected
 *
 * @param port_id
 *   The port identifier of the Ethernet device
 * @return
 *   The NUMA socket id to which the Ethernet device is connected or
 *   a default of zero if the socket could not be determined.
 *   -1 is returned is the port_id value is out of range.
 */
int rte_eth_dev_socket_id(uint16_t port_id);

/**
 * Check if port_id of device is attached
 *
 * @param port_id
 *   The port identifier of the Ethernet device
 * @return
 *   - 0 if port is out of range or not attached
 *   - 1 if device is attached
 */
int rte_eth_dev_is_valid_port(uint16_t port_id);

/**
 * Start specified RX queue of a port. It is used when rx_deferred_start
 * flag of the specified queue is true.
 *
 * @param port_id
 *   The port identifier of the Ethernet device
 * @param rx_queue_id
 *   The index of the rx queue to update the ring.
 *   The value must be in the range [0, nb_rx_queue - 1] previously supplied
 *   to rte_eth_dev_configure().
 * @return
 *   - 0: Success, the receive queue is started.
 *   - -EINVAL: The port_id or the queue_id out of range.
 *   - -EIO: if device is removed.
 *   - -ENOTSUP: The function not supported in PMD driver.
 */
int rte_eth_dev_rx_queue_start(uint16_t port_id, uint16_t rx_queue_id);

/**
 * Stop specified RX queue of a port
 *
 * @param port_id
 *   The port identifier of the Ethernet device
 * @param rx_queue_id
 *   The index of the rx queue to update the ring.
 *   The value must be in the range [0, nb_rx_queue - 1] previously supplied
 *   to rte_eth_dev_configure().
 * @return
 *   - 0: Success, the receive queue is stopped.
 *   - -EINVAL: The port_id or the queue_id out of range.
 *   - -EIO: if device is removed.
 *   - -ENOTSUP: The function not supported in PMD driver.
 */
int rte_eth_dev_rx_queue_stop(uint16_t port_id, uint16_t rx_queue_id);

/**
 * Start TX for specified queue of a port. It is used when tx_deferred_start
 * flag of the specified queue is true.
 *
 * @param port_id
 *   The port identifier of the Ethernet device
 * @param tx_queue_id
 *   The index of the tx queue to update the ring.
 *   The value must be in the range [0, nb_tx_queue - 1] previously supplied
 *   to rte_eth_dev_configure().
 * @return
 *   - 0: Success, the transmit queue is started.
 *   - -EINVAL: The port_id or the queue_id out of range.
 *   - -EIO: if device is removed.
 *   - -ENOTSUP: The function not supported in PMD driver.
 */
int rte_eth_dev_tx_queue_start(uint16_t port_id, uint16_t tx_queue_id);

/**
 * Stop specified TX queue of a port
 *
 * @param port_id
 *   The port identifier of the Ethernet device
 * @param tx_queue_id
 *   The index of the tx queue to update the ring.
 *   The value must be in the range [0, nb_tx_queue - 1] previously supplied
 *   to rte_eth_dev_configure().
 * @return
 *   - 0: Success, the transmit queue is stopped.
 *   - -EINVAL: The port_id or the queue_id out of range.
 *   - -EIO: if device is removed.
 *   - -ENOTSUP: The function not supported in PMD driver.
 */
int rte_eth_dev_tx_queue_stop(uint16_t port_id, uint16_t tx_queue_id);

/**
 * Start an Ethernet device.
 *
 * The device start step is the last one and consists of setting the configured
 * offload features and in starting the transmit and the receive units of the
 * device.
 *
 * Device RTE_ETH_DEV_NOLIVE_MAC_ADDR flag causes MAC address to be set before
 * PMD port start callback function is invoked.
 *
 * On success, all basic functions exported by the Ethernet API (link status,
 * receive/transmit, and so on) can be invoked.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @return
 *   - 0: Success, Ethernet device started.
 *   - <0: Error code of the driver device start function.
 */
int rte_eth_dev_start(uint16_t port_id);

/**
 * Stop an Ethernet device. The device can be restarted with a call to
 * rte_eth_dev_start()
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 */
void rte_eth_dev_stop(uint16_t port_id);

/**
 * Link up an Ethernet device.
 *
 * Set device link up will re-enable the device rx/tx
 * functionality after it is previously set device linked down.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @return
 *   - 0: Success, Ethernet device linked up.
 *   - <0: Error code of the driver device link up function.
 */
int rte_eth_dev_set_link_up(uint16_t port_id);

/**
 * Link down an Ethernet device.
 * The device rx/tx functionality will be disabled if success,
 * and it can be re-enabled with a call to
 * rte_eth_dev_set_link_up()
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 */
int rte_eth_dev_set_link_down(uint16_t port_id);

/**
 * Close a stopped Ethernet device. The device cannot be restarted!
 * The function frees all port resources if the driver supports
 * the flag RTE_ETH_DEV_CLOSE_REMOVE.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 */
void rte_eth_dev_close(uint16_t port_id);

/**
 * Reset a Ethernet device and keep its port id.
 *
 * When a port has to be reset passively, the DPDK application can invoke
 * this function. For example when a PF is reset, all its VFs should also
 * be reset. Normally a DPDK application can invoke this function when
 * RTE_ETH_EVENT_INTR_RESET event is detected, but can also use it to start
 * a port reset in other circumstances.
 *
 * When this function is called, it first stops the port and then calls the
 * PMD specific dev_uninit( ) and dev_init( ) to return the port to initial
 * state, in which no Tx and Rx queues are setup, as if the port has been
 * reset and not started. The port keeps the port id it had before the
 * function call.
 *
 * After calling rte_eth_dev_reset( ), the application should use
 * rte_eth_dev_configure( ), rte_eth_rx_queue_setup( ),
 * rte_eth_tx_queue_setup( ), and rte_eth_dev_start( )
 * to reconfigure the device as appropriate.
 *
 * Note: To avoid unexpected behavior, the application should stop calling
 * Tx and Rx functions before calling rte_eth_dev_reset( ). For thread
 * safety, all these controlling functions should be called from the same
 * thread.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 *
 * @return
 *   - (0) if successful.
 *   - (-EINVAL) if port identifier is invalid.
 *   - (-ENOTSUP) if hardware doesn't support this function.
 *   - (-EPERM) if not ran from the primary process.
 *   - (-EIO) if re-initialisation failed or device is removed.
 *   - (-ENOMEM) if the reset failed due to OOM.
 *   - (-EAGAIN) if the reset temporarily failed and should be retried later.
 */
int rte_eth_dev_reset(uint16_t port_id);

/**
 * Enable receipt in promiscuous mode for an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 */
void rte_eth_promiscuous_enable(uint16_t port_id);

/**
 * Disable receipt in promiscuous mode for an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 */
void rte_eth_promiscuous_disable(uint16_t port_id);

/**
 * Return the value of promiscuous mode for an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @return
 *   - (1) if promiscuous is enabled
 *   - (0) if promiscuous is disabled.
 *   - (-1) on error
 */
int rte_eth_promiscuous_get(uint16_t port_id);

/**
 * Enable the receipt of any multicast frame by an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 */
void rte_eth_allmulticast_enable(uint16_t port_id);

/**
 * Disable the receipt of all multicast frames by an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 */
void rte_eth_allmulticast_disable(uint16_t port_id);

/**
 * Return the value of allmulticast mode for an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @return
 *   - (1) if allmulticast is enabled
 *   - (0) if allmulticast is disabled.
 *   - (-1) on error
 */
int rte_eth_allmulticast_get(uint16_t port_id);

/**
 * Retrieve the status (ON/OFF), the speed (in Mbps) and the mode (HALF-DUPLEX
 * or FULL-DUPLEX) of the physical link of an Ethernet device. It might need
 * to wait up to 9 seconds in it.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param link
 *   A pointer to an *rte_eth_link* structure to be filled with
 *   the status, the speed and the mode of the Ethernet device link.
 */
void rte_eth_link_get(uint16_t port_id, struct rte_eth_link *link);

/**
 * Retrieve the status (ON/OFF), the speed (in Mbps) and the mode (HALF-DUPLEX
 * or FULL-DUPLEX) of the physical link of an Ethernet device. It is a no-wait
 * version of rte_eth_link_get().
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param link
 *   A pointer to an *rte_eth_link* structure to be filled with
 *   the status, the speed and the mode of the Ethernet device link.
 */
void rte_eth_link_get_nowait(uint16_t port_id, struct rte_eth_link *link);

/**
 * Retrieve the general I/O statistics of an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param stats
 *   A pointer to a structure of type *rte_eth_stats* to be filled with
 *   the values of device counters for the following set of statistics:
 *   - *ipackets* with the total of successfully received packets.
 *   - *opackets* with the total of successfully transmitted packets.
 *   - *ibytes*   with the total of successfully received bytes.
 *   - *obytes*   with the total of successfully transmitted bytes.
 *   - *ierrors*  with the total of erroneous received packets.
 *   - *oerrors*  with the total of failed transmitted packets.
 * @return
 *   Zero if successful. Non-zero otherwise.
 */
int rte_eth_stats_get(uint16_t port_id, struct rte_eth_stats *stats);

/**
 * Reset the general I/O statistics of an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @return
 *   - (0) if device notified to reset stats.
 *   - (-ENOTSUP) if hardware doesn't support.
 *   - (-ENODEV) if *port_id* invalid.
 */
int rte_eth_stats_reset(uint16_t port_id);

/**
 * Retrieve names of extended statistics of an Ethernet device.
 *
 * There is an assumption that 'xstat_names' and 'xstats' arrays are matched
 * by array index:
 *  xstats_names[i].name => xstats[i].value
 *
 * And the array index is same with id field of 'struct rte_eth_xstat':
 *  xstats[i].id == i
 *
 * This assumption makes key-value pair matching less flexible but simpler.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param xstats_names
 *   An rte_eth_xstat_name array of at least *size* elements to
 *   be filled. If set to NULL, the function returns the required number
 *   of elements.
 * @param size
 *   The size of the xstats_names array (number of elements).
 * @return
 *   - A positive value lower or equal to size: success. The return value
 *     is the number of entries filled in the stats table.
 *   - A positive value higher than size: error, the given statistics table
 *     is too small. The return value corresponds to the size that should
 *     be given to succeed. The entries in the table are not valid and
 *     shall not be used by the caller.
 *   - A negative value on error (invalid port id).
 */
int rte_eth_xstats_get_names(uint16_t port_id,
                             struct rte_eth_xstat_name *xstats_names,
                             unsigned int size);

/**
 * Retrieve extended statistics of an Ethernet device.
 *
 * There is an assumption that 'xstat_names' and 'xstats' arrays are matched
 * by array index:
 *  xstats_names[i].name => xstats[i].value
 *
 * And the array index is same with id field of 'struct rte_eth_xstat':
 *  xstats[i].id == i
 *
 * This assumption makes key-value pair matching less flexible but simpler.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param xstats
 *   A pointer to a table of structure of type *rte_eth_xstat*
 *   to be filled with device statistics ids and values.
 *   This parameter can be set to NULL if n is 0.
 * @param n
 *   The size of the xstats array (number of elements).
 * @return
 *   - A positive value lower or equal to n: success. The return value
 *     is the number of entries filled in the stats table.
 *   - A positive value higher than n: error, the given statistics table
 *     is too small. The return value corresponds to the size that should
 *     be given to succeed. The entries in the table are not valid and
 *     shall not be used by the caller.
 *   - A negative value on error (invalid port id).
 */
int rte_eth_xstats_get(uint16_t port_id, struct rte_eth_xstat *xstats,
                       unsigned int n);

/**
 * Retrieve names of extended statistics of an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param xstats_names
 *   An rte_eth_xstat_name array of at least *size* elements to
 *   be filled. If set to NULL, the function returns the required number
 *   of elements.
 * @param ids
 *   IDs array given by app to retrieve specific statistics
 * @param size
 *   The size of the xstats_names array (number of elements).
 * @return
 *   - A positive value lower or equal to size: success. The return value
 *     is the number of entries filled in the stats table.
 *   - A positive value higher than size: error, the given statistics table
 *     is too small. The return value corresponds to the size that should
 *     be given to succeed. The entries in the table are not valid and
 *     shall not be used by the caller.
 *   - A negative value on error (invalid port id).
 */
int
rte_eth_xstats_get_names_by_id(uint16_t port_id,
                               struct rte_eth_xstat_name *xstats_names, unsigned int size,
                               uint64_t *ids);

/**
 * Retrieve extended statistics of an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param ids
 *   A pointer to an ids array passed by application. This tells which
 *   statistics values function should retrieve. This parameter
 *   can be set to NULL if size is 0. In this case function will retrieve
 *   all avalible statistics.
 * @param values
 *   A pointer to a table to be filled with device statistics values.
 * @param size
 *   The size of the ids array (number of elements).
 * @return
 *   - A positive value lower or equal to size: success. The return value
 *     is the number of entries filled in the stats table.
 *   - A positive value higher than size: error, the given statistics table
 *     is too small. The return value corresponds to the size that should
 *     be given to succeed. The entries in the table are not valid and
 *     shall not be used by the caller.
 *   - A negative value on error (invalid port id).
 */
int rte_eth_xstats_get_by_id(uint16_t port_id, const uint64_t *ids,
                             uint64_t *values, unsigned int size);

/**
 * Gets the ID of a statistic from its name.
 *
 * This function searches for the statistics using string compares, and
 * as such should not be used on the fast-path. For fast-path retrieval of
 * specific statistics, store the ID as provided in *id* from this function,
 * and pass the ID to rte_eth_xstats_get()
 *
 * @param port_id The port to look up statistics from
 * @param xstat_name The name of the statistic to return
 * @param[out] id A pointer to an app-supplied uint64_t which should be
 *                set to the ID of the stat if the stat exists.
 * @return
 *    0 on success
 *    -ENODEV for invalid port_id,
 *    -EIO if device is removed,
 *    -EINVAL if the xstat_name doesn't exist in port_id
 */
int rte_eth_xstats_get_id_by_name(uint16_t port_id, const char *xstat_name,
                                  uint64_t *id);

/**
 * Reset extended statistics of an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 */
void rte_eth_xstats_reset(uint16_t port_id);

/**
 *  Set a mapping for the specified transmit queue to the specified per-queue
 *  statistics counter.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param tx_queue_id
 *   The index of the transmit queue for which a queue stats mapping is required.
 *   The value must be in the range [0, nb_tx_queue - 1] previously supplied
 *   to rte_eth_dev_configure().
 * @param stat_idx
 *   The per-queue packet statistics functionality number that the transmit
 *   queue is to be assigned.
 *   The value must be in the range [0, RTE_ETHDEV_QUEUE_STAT_CNTRS - 1].
 * @return
 *   Zero if successful. Non-zero otherwise.
 */
int rte_eth_dev_set_tx_queue_stats_mapping(uint16_t port_id,
                                           uint16_t tx_queue_id, uint8_t stat_idx);

/**
 *  Set a mapping for the specified receive queue to the specified per-queue
 *  statistics counter.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param rx_queue_id
 *   The index of the receive queue for which a queue stats mapping is required.
 *   The value must be in the range [0, nb_rx_queue - 1] previously supplied
 *   to rte_eth_dev_configure().
 * @param stat_idx
 *   The per-queue packet statistics functionality number that the receive
 *   queue is to be assigned.
 *   The value must be in the range [0, RTE_ETHDEV_QUEUE_STAT_CNTRS - 1].
 * @return
 *   Zero if successful. Non-zero otherwise.
 */
int rte_eth_dev_set_rx_queue_stats_mapping(uint16_t port_id,
                                           uint16_t rx_queue_id,
                                           uint8_t stat_idx);

/**
 * Retrieve the Ethernet address of an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param mac_addr
 *   A pointer to a structure of type *ether_addr* to be filled with
 *   the Ethernet address of the Ethernet device.
 */
void rte_eth_macaddr_get(uint16_t port_id, struct ether_addr *mac_addr);

/**
 * Retrieve the contextual information of an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param dev_info
 *   A pointer to a structure of type *rte_eth_dev_info* to be filled with
 *   the contextual information of the Ethernet device.
 */
void rte_eth_dev_info_get(uint16_t port_id, struct rte_eth_dev_info *dev_info);

/**
 * Retrieve the firmware version of a device.
 *
 * @param port_id
 *   The port identifier of the device.
 * @param fw_version
 *   A pointer to a string array storing the firmware version of a device,
 *   the string includes terminating null. This pointer is allocated by caller.
 * @param fw_size
 *   The size of the string array pointed by fw_version, which should be
 *   large enough to store firmware version of the device.
 * @return
 *   - (0) if successful.
 *   - (-ENOTSUP) if operation is not supported.
 *   - (-ENODEV) if *port_id* invalid.
 *   - (-EIO) if device is removed.
 *   - (>0) if *fw_size* is not enough to store firmware version, return
 *          the size of the non truncated string.
 */
int rte_eth_dev_fw_version_get(uint16_t port_id,
                               char *fw_version, size_t fw_size);

/**
 * Retrieve the supported packet types of an Ethernet device.
 *
 * When a packet type is announced as supported, it *must* be recognized by
 * the PMD. For instance, if RTE_PTYPE_L2_ETHER, RTE_PTYPE_L2_ETHER_VLAN
 * and RTE_PTYPE_L3_IPV4 are announced, the PMD must return the following
 * packet types for these packets:
 * - Ether/IPv4              -> RTE_PTYPE_L2_ETHER | RTE_PTYPE_L3_IPV4
 * - Ether/Vlan/IPv4         -> RTE_PTYPE_L2_ETHER_VLAN | RTE_PTYPE_L3_IPV4
 * - Ether/[anything else]   -> RTE_PTYPE_L2_ETHER
 * - Ether/Vlan/[anything else] -> RTE_PTYPE_L2_ETHER_VLAN
 *
 * When a packet is received by a PMD, the most precise type must be
 * returned among the ones supported. However a PMD is allowed to set
 * packet type that is not in the supported list, at the condition that it
 * is more precise. Therefore, a PMD announcing no supported packet types
 * can still set a matching packet type in a received packet.
 *
 * @note
 *   Better to invoke this API after the device is already started or rx burst
 *   function is decided, to obtain correct supported ptypes.
 * @note
 *   if a given PMD does not report what ptypes it supports, then the supported
 *   ptype count is reported as 0.
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param ptype_mask
 *   A hint of what kind of packet type which the caller is interested in.
 * @param ptypes
 *   An array pointer to store adequate packet types, allocated by caller.
 * @param num
 *  Size of the array pointed by param ptypes.
 * @return
 *   - (>=0) Number of supported ptypes. If the number of types exceeds num,
 *           only num entries will be filled into the ptypes array, but the full
 *           count of supported ptypes will be returned.
 *   - (-ENODEV) if *port_id* invalid.
 */
int rte_eth_dev_get_supported_ptypes(uint16_t port_id, uint32_t ptype_mask,
                                     uint32_t *ptypes, int num);

/**
 * Retrieve the MTU of an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param mtu
 *   A pointer to a uint16_t where the retrieved MTU is to be stored.
 * @return
 *   - (0) if successful.
 *   - (-ENODEV) if *port_id* invalid.
 */
int rte_eth_dev_get_mtu(uint16_t port_id, uint16_t *mtu);

/**
 * Change the MTU of an Ethernet device.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param mtu
 *   A uint16_t for the MTU to be applied.
 * @return
 *   - (0) if successful.
 *   - (-ENOTSUP) if operation is not supported.
 *   - (-ENODEV) if *port_id* invalid.
 *   - (-EIO) if device is removed.
 *   - (-EINVAL) if *mtu* invalid.
 *   - (-EBUSY) if operation is not allowed when the port is running
 */
int rte_eth_dev_set_mtu(uint16_t port_id, uint16_t mtu);

/**
 * Enable/Disable hardware filtering by an Ethernet device of received
 * VLAN packets tagged with a given VLAN Tag Identifier.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param vlan_id
 *   The VLAN Tag Identifier whose filtering must be enabled or disabled.
 * @param on
 *   If > 0, enable VLAN filtering of VLAN packets tagged with *vlan_id*.
 *   Otherwise, disable VLAN filtering of VLAN packets tagged with *vlan_id*.
 * @return
 *   - (0) if successful.
 *   - (-ENOSUP) if hardware-assisted VLAN filtering not configured.
 *   - (-ENODEV) if *port_id* invalid.
 *   - (-EIO) if device is removed.
 *   - (-ENOSYS) if VLAN filtering on *port_id* disabled.
 *   - (-EINVAL) if *vlan_id* > 4095.
 */
int rte_eth_dev_vlan_filter(uint16_t port_id, uint16_t vlan_id, int on);

/**
 * Enable/Disable hardware VLAN Strip by a rx queue of an Ethernet device.
 * 82599/X540/X550 can support VLAN stripping at the rx queue level
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param rx_queue_id
 *   The index of the receive queue for which a queue stats mapping is required.
 *   The value must be in the range [0, nb_rx_queue - 1] previously supplied
 *   to rte_eth_dev_configure().
 * @param on
 *   If 1, Enable VLAN Stripping of the receive queue of the Ethernet port.
 *   If 0, Disable VLAN Stripping of the receive queue of the Ethernet port.
 * @return
 *   - (0) if successful.
 *   - (-ENOSUP) if hardware-assisted VLAN stripping not configured.
 *   - (-ENODEV) if *port_id* invalid.
 *   - (-EINVAL) if *rx_queue_id* invalid.
 */
int rte_eth_dev_set_vlan_strip_on_queue(uint16_t port_id, uint16_t rx_queue_id,
                                        int on);

/**
 * Set the Outer VLAN Ether Type by an Ethernet device, it can be inserted to
 * the VLAN Header. This is a register setup available on some Intel NIC, not
 * but all, please check the data sheet for availability.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param vlan_type
 *   The vlan type.
 * @param tag_type
 *   The Tag Protocol ID
 * @return
 *   - (0) if successful.
 *   - (-ENOSUP) if hardware-assisted VLAN TPID setup is not supported.
 *   - (-ENODEV) if *port_id* invalid.
 *   - (-EIO) if device is removed.
 */
int rte_eth_dev_set_vlan_ether_type(uint16_t port_id,
                                    enum rte_vlan_type vlan_type,
                                    uint16_t tag_type);

/**
 * Set VLAN offload configuration on an Ethernet device
 * Enable/Disable Extended VLAN by an Ethernet device, This is a register setup
 * available on some Intel NIC, not but all, please check the data sheet for
 * availability.
 * Enable/Disable VLAN Strip can be done on rx queue for certain NIC, but here
 * the configuration is applied on the port level.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param offload_mask
 *   The VLAN Offload bit mask can be mixed use with "OR"
 *       ETH_VLAN_STRIP_OFFLOAD
 *       ETH_VLAN_FILTER_OFFLOAD
 *       ETH_VLAN_EXTEND_OFFLOAD
 * @return
 *   - (0) if successful.
 *   - (-ENOSUP) if hardware-assisted VLAN filtering not configured.
 *   - (-ENODEV) if *port_id* invalid.
 *   - (-EIO) if device is removed.
 */
int rte_eth_dev_set_vlan_offload(uint16_t port_id, int offload_mask);

/**
 * Read VLAN Offload configuration from an Ethernet device
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @return
 *   - (>0) if successful. Bit mask to indicate
 *       ETH_VLAN_STRIP_OFFLOAD
 *       ETH_VLAN_FILTER_OFFLOAD
 *       ETH_VLAN_EXTEND_OFFLOAD
 *   - (-ENODEV) if *port_id* invalid.
 */
int rte_eth_dev_get_vlan_offload(uint16_t port_id);

/**
 * Set port based TX VLAN insertion on or off.
 *
 * @param port_id
 *  The port identifier of the Ethernet device.
 * @param pvid
 *  Port based TX VLAN identifier together with user priority.
 * @param on
 *  Turn on or off the port based TX VLAN insertion.
 *
 * @return
 *   - (0) if successful.
 *   - negative if failed.
 */
int rte_eth_dev_set_vlan_pvid(uint16_t port_id, uint16_t pvid, int on);

typedef void (*buffer_tx_error_fn)(struct rte_mbuf **unsent, uint16_t count,
                                   void *userdata);

/**
 * Structure used to buffer packets for future TX
 * Used by APIs rte_eth_tx_buffer and rte_eth_tx_buffer_flush
 */
struct rte_eth_dev_tx_buffer {
    buffer_tx_error_fn error_callback;
    void *error_userdata;
    uint16_t size;           /**< Size of buffer for buffered tx */
    uint16_t length;         /**< Number of packets in the array */
    struct rte_mbuf *pkts[];
    /**< Pending packets to be sent on explicit flush or when full */
};

/**
 * Calculate the size of the tx buffer.
 *
 * @param sz
 *   Number of stored packets.
 */
#define RTE_ETH_TX_BUFFER_SIZE(sz) \
	(sizeof(struct rte_eth_dev_tx_buffer) + (sz) * sizeof(struct rte_mbuf *))

/**
 * Initialize default values for buffered transmitting
 *
 * @param buffer
 *   Tx buffer to be initialized.
 * @param size
 *   Buffer size
 * @return
 *   0 if no error
 */
int
rte_eth_tx_buffer_init(struct rte_eth_dev_tx_buffer *buffer, uint16_t size);

/**
 * Configure a callback for buffered packets which cannot be sent
 *
 * Register a specific callback to be called when an attempt is made to send
 * all packets buffered on an ethernet port, but not all packets can
 * successfully be sent. The callback registered here will be called only
 * from calls to rte_eth_tx_buffer() and rte_eth_tx_buffer_flush() APIs.
 * The default callback configured for each queue by default just frees the
 * packets back to the calling mempool. If additional behaviour is required,
 * for example, to count dropped packets, or to retry transmission of packets
 * which cannot be sent, this function should be used to register a suitable
 * callback function to implement the desired behaviour.
 * The example callback "rte_eth_count_unsent_packet_callback()" is also
 * provided as reference.
 *
 * @param buffer
 *   The port identifier of the Ethernet device.
 * @param callback
 *   The function to be used as the callback.
 * @param userdata
 *   Arbitrary parameter to be passed to the callback function
 * @return
 *   0 on success, or -1 on error with rte_errno set appropriately
 */
int
rte_eth_tx_buffer_set_err_callback(struct rte_eth_dev_tx_buffer *buffer,
                                   buffer_tx_error_fn callback, void *userdata);

/**
 * Callback function for silently dropping unsent buffered packets.
 *
 * This function can be passed to rte_eth_tx_buffer_set_err_callback() to
 * adjust the default behavior when buffered packets cannot be sent. This
 * function drops any unsent packets silently and is used by tx buffered
 * operations as default behavior.
 *
 * NOTE: this function should not be called directly, instead it should be used
 *       as a callback for packet buffering.
 *
 * NOTE: when configuring this function as a callback with
 *       rte_eth_tx_buffer_set_err_callback(), the final, userdata parameter
 *       should point to an uint64_t value.
 *
 * @param pkts
 *   The previously buffered packets which could not be sent
 * @param unsent
 *   The number of unsent packets in the pkts array
 * @param userdata
 *   Not used
 */
void
rte_eth_tx_buffer_drop_callback(struct rte_mbuf **pkts, uint16_t unsent,
                                void *userdata);

/**
 * Callback function for tracking unsent buffered packets.
 *
 * This function can be passed to rte_eth_tx_buffer_set_err_callback() to
 * adjust the default behavior when buffered packets cannot be sent. This
 * function drops any unsent packets, but also updates a user-supplied counter
 * to track the overall number of packets dropped. The counter should be an
 * uint64_t variable.
 *
 * NOTE: this function should not be called directly, instead it should be used
 *       as a callback for packet buffering.
 *
 * NOTE: when configuring this function as a callback with
 *       rte_eth_tx_buffer_set_err_callback(), the final, userdata parameter
 *       should point to an uint64_t value.
 *
 * @param pkts
 *   The previously buffered packets which could not be sent
 * @param unsent
 *   The number of unsent packets in the pkts array
 * @param userdata
 *   Pointer to an uint64_t value, which will be incremented by unsent
 */
void
rte_eth_tx_buffer_count_callback(struct rte_mbuf **pkts, uint16_t unsent,
                                 void *userdata);

/**
 * Request the driver to free mbufs currently cached by the driver. The
 * driver will only free the mbuf if it is no longer in use. It is the
 * application's responsibity to ensure rte_eth_tx_buffer_flush(..) is
 * called if needed.
 *
 * @param port_id
 *   The port identifier of the Ethernet device.
 * @param queue_id
 *   The index of the transmit queue through which output packets must be
 *   sent.
 *   The value must be in the range [0, nb_tx_queue - 1] previously supplied
 *   to rte_eth_dev_configure().
 * @param free_cnt
 *   Maximum number of packets to free. Use 0 to indicate all possible packets
 *   should be freed. Note that a packet may be using multiple mbufs.
 * @return
 *   Failure: < 0
 *     -ENODEV: Invalid interface
 *     -EIO: device is removed
 *     -ENOTSUP: Driver does not support function
 *   Success: >= 0
 *     0-n: Number of packets freed. More packets may still remain in ring that
 *     are in use.
 */
int
rte_eth_tx_done_cleanup(uint16_t port_id, uint16_t queue_id, uint32_t free_cnt);

#endif /* _RTE_ETHDEV_H_ */