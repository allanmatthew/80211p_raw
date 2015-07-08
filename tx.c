#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "rawsock_util.h"
#include "nl.h"

#define PAYLOAD_LEN 32
#define ETH_TYPE_3DR (0x88DD)
#define CHANNEL 3

char* if_name = "wlan0";

int main (void)
{
  int raw_socket = -1;
  struct sockaddr_ll sockaddr;
  uint8_t *packet = 0;
  uint8_t *packet_ptr;
  int packet_len;
  uint8_t *payload = 0;
  eth_hdr_t eth_hdr;
  int n_bytes_sent = 0;
  int ret;
  int i;

  //Set up the netlink socket
  nl_set_channel(CHANNEL,5);

  // Open the raw socket
  ret = open_socket(if_name, &raw_socket, &sockaddr);
  if (ret < 0){
    printf("Error opening socket: %i\n", raw_socket);
    return -1;
  }

  // Create the packet
  packet_len = PAYLOAD_LEN + sizeof(eth_hdr_t);
  packet = malloc(packet_len);
  if ( packet == 0 ) {
    printf("Unable to allocate packet memory\n");
  }

  packet_ptr = packet;

  // Add the ethernet header
  populate_eth_hdr(&eth_hdr, sockaddr.sll_addr, ETH_TYPE_3DR);
  memcpy(packet_ptr, &eth_hdr, sizeof(eth_hdr_t));
  packet_ptr += sizeof(eth_hdr_t);

  // Populate the payload
  payload = malloc(PAYLOAD_LEN);
  if (payload == 0) {
    printf("Unable to allocate payload\n");
    return -1;
  }
  memset(payload, 0, PAYLOAD_LEN);
  memcpy(packet_ptr, payload, PAYLOAD_LEN);

  i=0;
  while (2) {
    //Set the sequence number in the packet
    memcpy(packet + sizeof(eth_hdr_t),&i,4);
    ++i;

    printf("Sending on channel %i", CHANNEL);
    printf("seq: %i\n",i);

    n_bytes_sent = sendto(raw_socket, packet, packet_len, 0, 
        (const struct sockaddr *)&sockaddr, sizeof(struct sockaddr_ll));
    if (n_bytes_sent <= 0) {
      printf("Unable to send data: %s\n", strerror(errno));
      return -1;
    }
  }
  return 0;
}
