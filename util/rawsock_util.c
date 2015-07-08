#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <errno.h>
#include "rawsock_util.h"

/*
 * Opens a socket for raw communications.  Packets are broadcast type
 */
int open_socket(char *interface_name, int *raw_socket, struct sockaddr_ll *sockaddr)
{

  int if_index = -1;
  struct ifreq if_mac;

  // open a raw socket
  *raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (*raw_socket <0) {
    printf("Could not open socket\n");
    return -EPERM;
  }

  //Get the interface index
  if ((if_index = if_nametoindex(interface_name)) == 0) {
    printf("Could not get interface index\n");
    return -ENODEV;
  }

  //Get the interface MAC
  memset(&if_mac, 0, sizeof(if_mac));
  strncpy(if_mac.ifr_name, interface_name, IFNAMSIZ-1);
  if (ioctl(*raw_socket, SIOCGIFHWADDR, &if_mac) < 0) {
    return -EACCES;
  }

  /* Populate sockaddr */
  memset(sockaddr, 0, sizeof(struct sockaddr_ll));
  sockaddr->sll_family = AF_PACKET;
  sockaddr->sll_protocol = htons(ETH_P_ALL);
  sockaddr->sll_ifindex = if_index;
  sockaddr->sll_hatype = 0; /* ARPHRD_IEEE80211 */
  sockaddr->sll_pkttype = PACKET_BROADCAST; /* PACKET_BROADCAST */
  sockaddr->sll_halen = IFHWADDRLEN;
  sockaddr->sll_addr[0] = if_mac.ifr_hwaddr.sa_data[0];
  sockaddr->sll_addr[1] = if_mac.ifr_hwaddr.sa_data[1];
  sockaddr->sll_addr[2] = if_mac.ifr_hwaddr.sa_data[2];
  sockaddr->sll_addr[3] = if_mac.ifr_hwaddr.sa_data[3];
  sockaddr->sll_addr[4] = if_mac.ifr_hwaddr.sa_data[4];
  sockaddr->sll_addr[5] = if_mac.ifr_hwaddr.sa_data[5];

  return 0;
}

/*
 * Populates a header with eth data for packet transmission.
 * This creates a header with a broadcast address.
 */
void populate_eth_hdr(eth_hdr_t *hdr, unsigned char my_addr[], uint16_t ether_type)
{
  memset(hdr, 0, sizeof(eth_hdr_t));
  hdr->ether_shost[0] = my_addr[0];
  hdr->ether_shost[1] = my_addr[1];
  hdr->ether_shost[2] = my_addr[2];
  hdr->ether_shost[3] = my_addr[3];
  hdr->ether_shost[4] = my_addr[4];
  hdr->ether_shost[5] = my_addr[5];

  /* Use broadcast destination address */
  hdr->ether_dhost[0] = 0xFF;
  hdr->ether_dhost[1] = 0xFF;
  hdr->ether_dhost[2] = 0xFF;
  hdr->ether_dhost[3] = 0xFF;
  hdr->ether_dhost[4] = 0xFF;
  hdr->ether_dhost[5] = 0xFF;

  hdr->ether_type = htons(ether_type);
}

