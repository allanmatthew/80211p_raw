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
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include "rawsock_util.h"
#include "nl.h"

#define PAYLOAD_LEN 1518
#define ETH_TYPE_3DR (0x88DD)
#define NUM_RETRANSMITS 1

char* if_name = "wlan0";

int main (void)
{
  int raw_socket = -1;
  int udp_socket = -1;
  struct sockaddr_in udp_sockaddr;
  struct sockaddr_ll sockaddr;
  uint8_t *packet = 0;
  uint8_t *packet_ptr;
  int packet_len;
  uint8_t *payload = 0;
  eth_hdr_t eth_hdr;
  int n_bytes_sent = 0;
  int ret;
  int j;
  uint8_t packet_buffer[ETH_FRAME_LEN];
  int addrlen = -1;
  int bytes_received = 0;

  //Set up the netlink socket
  int chan = 3;
  nl_set_channel(chan,5);

  // Open the raw socket
  ret = open_socket(if_name, &raw_socket, &sockaddr);
  if (ret < 0){
    printf("Error opening socket: %i\n", raw_socket);
    return -1;
  }

  // Open the udp socket
  udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_socket < 0){
    printf("Error opening udp socket: %i\n", udp_socket);
    return -1;
  }
  //fcntl(udp_socket, F_SETFL, O_NONBLOCK);  // set to non-blocking

  // Bind the socket to port 5580
  memset((char *)&udp_sockaddr, 0, sizeof(udp_sockaddr));
  udp_sockaddr.sin_family = AF_INET;
  udp_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  udp_sockaddr.sin_port = htons(5550);
  ret = bind(udp_socket, (struct sockaddr *)&udp_sockaddr, sizeof(struct sockaddr_in));
  if (ret < 0) {
    printf("Unable to bind to udp socket: %s\n", strerror(errno));
    return -1;
  }

  // Create the packet
  //packet_len = PAYLOAD_LEN + sizeof(eth_hdr_t);
  packet_len = ETH_FRAME_LEN;
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
  
  while (1) {
    //Clear the packet buffer
    memset(packet_buffer, 0, ETH_FRAME_LEN);

    //Read from the incoming port.
    addrlen=sizeof(udp_sockaddr);
    bytes_received = recvfrom(udp_socket, packet_buffer, sizeof(packet_buffer), 0, 
                              (struct sockaddr*)&udp_sockaddr, (socklen_t*) &addrlen);

    if ( bytes_received < 0 ) {
      printf("error: %s\n", strerror(errno));
      continue;
    }

    //Copy the rest of the recieved packet into the sending packet
    memcpy(packet + sizeof(eth_hdr_t), packet_buffer, bytes_received);

    packet_len = sizeof(eth_hdr_t) + bytes_received;

    printf("Sending %i bytes on channel %i\n", packet_len, chan);

    //Send the same packet NUM_RETRANSMITS times
    for(j=0; j<NUM_RETRANSMITS; ++j){

      n_bytes_sent = sendto(raw_socket, packet, packet_len, 0, 
                            (const struct sockaddr *)&sockaddr, sizeof(struct sockaddr_ll));
      if (n_bytes_sent <= 0) {
        printf("Unable to send data: %s\n", strerror(errno));
        return -1;
      }
    }

  }
  return 0;
}
