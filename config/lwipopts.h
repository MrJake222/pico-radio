#ifndef _LWIPOPTS_EXAMPLE_COMMONH_H
#define _LWIPOPTS_EXAMPLE_COMMONH_H

#include "FreeRTOSConfig.h"
#include "config.hpp"

#define NO_SYS                      0
#define LWIP_SOCKET                 0

#define TCPIP_THREAD_NAME           "tcpip"
#define TCPIP_THREAD_PRIO           (configMAX_PRIORITIES - 1)
#define TCPIP_THREAD_STACKSIZE      2048

// data needs to be received fast for this to work
// set high tcpip task priority
#define LWIP_DISABLE_TCP_SANITY_CHECKS  1
#define PBUF_POOL_SIZE                  16 // unit is ~ethernet packet (1500 bytes), whole window is 43
#define TCP_WND                         (63 * 1024) // was around 11, then 24
                                                    // now 63 (can't be 64 because of TCP window being a 16-bit number with no scaling)

#define DEFAULT_TCP_RECVMBOX_SIZE   5
#define DEFAULT_ACCEPTMBOX_SIZE     5
#define TCPIP_MBOX_SIZE			 	10

#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_ARP_QUEUE          10
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    0
#define TCP_MSS                     1460
#define TCP_SND_BUF                 4096
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
// #define TCP_FAST_INTERVAL           50
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETCONN                0
#define MEM_STATS                   0
#define SYS_STATS                   0
#define MEMP_STATS                  0
#define LINK_STATS                  0
// #define ETH_PAD_SIZE                2
#define LWIP_CHKSUM_ALGORITHM       3
#define LWIP_DHCP                   1
#define LWIP_IPV4                   1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DNS                    1
#define LWIP_TCP_KEEPALIVE          1
#define LWIP_NETIF_TX_SINGLE_PBUF   1
#define DHCP_DOES_ARP_CHECK         0
#define LWIP_DHCP_DOES_ACD_CHECK    0

#define DEBUG                       0
#if DEBUG
#define LWIP_DEBUG                  1
#define LWIP_STATS                  1
#define LWIP_STATS_DISPLAY          1
#define UDP_STATS                   0
#define ICMP_STATS                  0
#define MEM_STATS                   1
#define MEMP_STATS                  0
#define LINK_STATS                  0
#define IPFRAG_STATS                0
#endif

#define DEBUG_MEM                   0
#if DEBUG_MEM
#define LWIP_DEBUG                  1
#define LWIP_STATS                  1
#define LWIP_STATS_DISPLAY          1
#define MEM_STATS                   0
#define MEMP_STATS                  1
#define ETHARP_STATS                0
#define TCP_STATS                   0
#define IP_STATS                    0
#define UDP_STATS                   0
#define ICMP_STATS                  0
#define LINK_STATS                  0
#define IPFRAG_STATS                0
#endif

#define ETHARP_DEBUG                LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define INET_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define PPP_DEBUG                   LWIP_DBG_OFF
#define SLIP_DEBUG                  LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF

#endif /* __LWIPOPTS_H__ */
