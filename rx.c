#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
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
#include "rawsock_util.h"
#include "nl.h"

#define PAYLOAD_LEN 32 
#define ETH_TYPE_3DR (0x88DD)
#define CHANNEL 3

char* if_name = "wlan0";
/*
 * Simple example receiving packets with a sequence number in them
 */
int main (void)
{
  int raw_socket = -1;
  struct sockaddr_ll sockaddr;
  int ret;
  uint8_t packet_buffer[ETH_FRAME_LEN];
  uint8_t payload[ETH_DATA_LEN];
  int bytes_received = -1;
  int addrlen = -1;
  int seq;
  int lastseq = 0;

  //Set the channel to 3 to begin with
  nl_set_channel(CHANNEL,5);

  // Open the raw socket
  ret = open_socket(if_name, &raw_socket, &sockaddr);
  if (ret < 0){
    printf("Error opening socket: %i\n", raw_socket);
    return -1;
  }

  fcntl(raw_socket, F_SETFL, O_NONBLOCK);  // set to non-blocking

  // Bind to the socket
  ret = bind(raw_socket, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_ll));
  if (ret < 0) {
    printf("Unable to bind to socket: %s\n", strerror(errno));
    return -1;
  }

  memset(packet_buffer, 0, sizeof(packet_buffer));
  addrlen = sizeof(sockaddr);

  while (1) {
    bytes_received = recvfrom(raw_socket, packet_buffer, sizeof(packet_buffer), 0, 
        (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
    if (bytes_received < 0) {
      continue;
    }
    else
    {
      memset(payload,0,sizeof(payload));
      memcpy(payload, (packet_buffer + sizeof(eth_hdr_t)), bytes_received);

      memcpy(&seq, payload, 4);

      printf("sequence number: %i", seq);
      if(seq - lastseq > 1)
        printf(" missed %i\n", (seq-lastseq-1));
      else if (seq == lastseq)
        printf(" (dup)\n");
      else
        printf("\n");

      lastseq=seq;
    }
  }
  return 0;
}
