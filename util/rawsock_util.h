#ifndef __RAWSOCK_UTIL_H__
#define __RAWSOCK_UTIL_H__
#define PACKED __attribute__ ((packed))

typedef struct eth_hdr {
    uint8_t  ether_dhost[ETH_ALEN];
    uint8_t  ether_shost[ETH_ALEN];
    uint16_t ether_type;
} PACKED eth_hdr_t;

int open_socket(char *interface_name, int *raw_socket, struct sockaddr_ll *sockaddr);
void populate_eth_hdr(eth_hdr_t *hdr, unsigned char my_addr[], uint16_t ether_type);
#endif //__RAWSOCK_UTIL_H__
