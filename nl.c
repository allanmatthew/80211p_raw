#include <stdio.h>
#include <net/if.h>
#include <netlink/netlink.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include "nl.h"
#include "nl80211_copy.h"

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
    void *arg);
static int finish_handler(struct nl_msg *msg, void *arg);
static int ack_handler(struct nl_msg *msg, void *arg);

int nl_set_channel(int channel, int width)
{
  int ret;
  ret = nl_leave_ocb();
  ret = nl_join_ocb(channel, width);
  //ret = nl_set_ocb_channel(channel, width);
  return ret;
}

int nl_set_ocb_channel(int channel, int width)
{
  struct nl_sock *nl_sock = NULL;
  int nl80211_id = -1;
  unsigned int if_index = 0;
  uint32_t if_index_uint32;
  struct nl_msg *msg = NULL;
  struct nl_cb *cmd_cb = NULL;
  struct nl_cb *sock_cb = NULL;
  void *hdr = NULL;
  int num_bytes = -1;
  int status = 0;

  printf("Attempting to set ocb channel %i width %i\n", channel, width);

  nl_sock = nl_socket_alloc();
  if (nl_sock == NULL) {
    fprintf(stderr, "ERROR allocating netlink socket\n");
    status = 1;
    goto cleanup;
  }

  if (genl_connect(nl_sock) != 0) {
    fprintf(stderr, "ERROR connecting netlink socket\n");
    status = 1;
    goto cleanup;
  }

  nl80211_id = genl_ctrl_resolve(nl_sock, "nl80211");
  if (nl80211_id < 0) {
    fprintf(stderr, "ERROR resolving netlink socket\n");
    status = 1;
    goto cleanup;
  }

  if_index = if_nametoindex("wlan0");
  if (if_index == 0) {
    fprintf(stderr, "ERROR getting interface index\n");
    status = 1;
    goto cleanup;
  }

  msg = nlmsg_alloc();
  if (msg == NULL) {
    fprintf(stderr, "ERROR allocating netlink message\n");
    status = 1;
    goto cleanup;
  }

  cmd_cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (cmd_cb == NULL) {
    fprintf(stderr, "ERROR allocating netlink command callback\n");
    status = 1;
    goto cleanup;
  }

  sock_cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (sock_cb == NULL) {
    fprintf(stderr, "ERROR allocating netlink socket callback\n");
    status = 1;
    goto cleanup;
  }

  // setup the message
  hdr = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0,
      0, NL80211_CMD_CHANGE_CHANNEL_OCB, 0);
  if (hdr == NULL) {
    fprintf(stderr, "ERROR creating netlink message\n");
    status = 1;
    goto cleanup;
  }

  //add message attributes
  if_index_uint32 = if_index;
  if (nla_put(msg, NL80211_ATTR_IFINDEX, sizeof(uint32_t), &if_index_uint32)
      != 0) {
    fprintf(stderr, "ERROR setting ifindex attribute\n");
    status = 1;
    goto cleanup;
  }

  uint32_t freq;
  if (channel >= 1 && channel <= 11)
    freq = (2412 + 5*(channel-1));
  else {
    printf("Incorrect channel, using channel 1\n");
    freq = 2412;
  }
  if (nla_put(msg, NL80211_ATTR_WIPHY_FREQ, sizeof(uint32_t), &freq)
      != 0) {
    fprintf(stderr, "ERROR setting freq attribute\n");
    status = 1;
    goto cleanup;
  }

  uint32_t _width;
  if (width == 5)
    _width = NL80211_CHAN_WIDTH_5;
  else if (width == 10)
    _width = NL80211_CHAN_WIDTH_10;
  else {
    printf("Channel width must be 5 or 10, using 10\n");
    _width = NL80211_CHAN_WIDTH_10;
  }
  if (nla_put(msg, NL80211_ATTR_CHANNEL_WIDTH, sizeof(uint32_t), &_width)
      != 0) {
    fprintf(stderr, "ERROR setting width attribute\n");
    status = 1;
    goto cleanup;
  }

  //printf("Sending netlink message\n");
  //send the messge (this frees it)
  num_bytes = nl_send_auto_complete(nl_sock, msg);
  if (num_bytes < 0) {
    fprintf(stderr, "ERROR sending netlink message\n");
    status = 1;
    goto cleanup;
  }

  status = 1;
  nl_cb_err(cmd_cb, NL_CB_CUSTOM, error_handler, (void *)&status);
  nl_cb_set(cmd_cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler,
      (void *)&status);
  nl_cb_set(cmd_cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, (void *)&status);

  //block for message to return
  nl_recvmsgs(nl_sock, cmd_cb);

cleanup:

  if (sock_cb != NULL)
    nl_cb_put(sock_cb);

  if (cmd_cb != NULL)
    nl_cb_put(cmd_cb);

  if (msg != NULL)
    nlmsg_free(msg);

  if (nl_sock != NULL)
    nl_socket_free(nl_sock);

  return 0;

}

int nl_leave_ocb(void)
{
  struct nl_sock *nl_sock = NULL;
  int nl80211_id = -1;
  unsigned int if_index = 0;
  uint32_t if_index_uint32;
  struct nl_msg *msg = NULL;
  struct nl_cb *cmd_cb = NULL;
  struct nl_cb *sock_cb = NULL;
  void *hdr = NULL;
  int num_bytes = -1;
  int status = 0;

  nl_sock = nl_socket_alloc();
  if (nl_sock == NULL) {
    fprintf(stderr, "ERROR allocating netlink socket\n");
    status = 1;
    goto cleanup;
  }

  if (genl_connect(nl_sock) != 0) {
    fprintf(stderr, "ERROR connecting netlink socket\n");
    status = 1;
    goto cleanup;
  }

  nl80211_id = genl_ctrl_resolve(nl_sock, "nl80211");
  if (nl80211_id < 0) {
    fprintf(stderr, "ERROR resolving netlink socket\n");
    status = 1;
    goto cleanup;
  }

  if_index = if_nametoindex("wlan0");
  if (if_index == 0) {
    fprintf(stderr, "ERROR getting interface index\n");
    status = 1;
    goto cleanup;
  }

  msg = nlmsg_alloc();
  if (msg == NULL) {
    fprintf(stderr, "ERROR allocating netlink message\n");
    status = 1;
    goto cleanup;
  }

  cmd_cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (cmd_cb == NULL) {
    fprintf(stderr, "ERROR allocating netlink command callback\n");
    status = 1;
    goto cleanup;
  }

  sock_cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (sock_cb == NULL) {
    fprintf(stderr, "ERROR allocating netlink socket callback\n");
    status = 1;
    goto cleanup;
  }

  //Leave the OCB first
  // setup the message
  hdr = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0,
      0, NL80211_CMD_LEAVE_OCB, 0);
  if (hdr == NULL) {
    fprintf(stderr, "ERROR creating netlink message\n");
    status = 1;
    goto cleanup;
  }

  if_index_uint32 = if_index;
  if (nla_put(msg, NL80211_ATTR_IFINDEX, sizeof(uint32_t), &if_index_uint32)
      != 0) {
    fprintf(stderr, "ERROR setting ifindex attribute\n");
    status = 1;
    goto cleanup;
  }

  //printf("Sending netlink message\n");
  //send the messge (this frees it)
  num_bytes = nl_send_auto_complete(nl_sock, msg);
  if (num_bytes < 0) {
    fprintf(stderr, "ERROR sending netlink message\n");
    status = 1;
    goto cleanup;
  }

  status = 1;
  nl_cb_err(cmd_cb, NL_CB_CUSTOM, error_handler, (void *)&status);
  nl_cb_set(cmd_cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler,
      (void *)&status);
  nl_cb_set(cmd_cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, (void *)&status);

  //block for message to return
  nl_recvmsgs(nl_sock, cmd_cb);

cleanup:

  if (sock_cb != NULL)
    nl_cb_put(sock_cb);

  if (cmd_cb != NULL)
    nl_cb_put(cmd_cb);

  if (msg != NULL)
    nlmsg_free(msg);

  if (nl_sock != NULL)
    nl_socket_free(nl_sock);

  return 0;

}

int nl_join_ocb(int channel, int width)
{
  struct nl_sock *nl_sock = NULL;
  int nl80211_id = -1;
  unsigned int if_index = 0;
  uint32_t if_index_uint32;
  struct nl_msg *msg = NULL;
  struct nl_cb *cmd_cb = NULL;
  struct nl_cb *sock_cb = NULL;
  void *hdr = NULL;
  int num_bytes = -1;
  int status = 0;

  printf("Attempting to join ocb channel %i width %i\n", channel, width);

  nl_sock = nl_socket_alloc();
  if (nl_sock == NULL) {
    fprintf(stderr, "ERROR allocating netlink socket\n");
    status = 1;
    goto cleanup;
  }

  if (genl_connect(nl_sock) != 0) {
    fprintf(stderr, "ERROR connecting netlink socket\n");
    status = 1;
    goto cleanup;
  }

  nl80211_id = genl_ctrl_resolve(nl_sock, "nl80211");
  if (nl80211_id < 0) {
    fprintf(stderr, "ERROR resolving netlink socket\n");
    status = 1;
    goto cleanup;
  }

  if_index = if_nametoindex("wlan0");
  if (if_index == 0) {
    fprintf(stderr, "ERROR getting interface index\n");
    status = 1;
    goto cleanup;
  }

  msg = nlmsg_alloc();
  if (msg == NULL) {
    fprintf(stderr, "ERROR allocating netlink message\n");
    status = 1;
    goto cleanup;
  }

  cmd_cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (cmd_cb == NULL) {
    fprintf(stderr, "ERROR allocating netlink command callback\n");
    status = 1;
    goto cleanup;
  }

  sock_cb = nl_cb_alloc(NL_CB_DEFAULT);
  if (sock_cb == NULL) {
    fprintf(stderr, "ERROR allocating netlink socket callback\n");
    status = 1;
    goto cleanup;
  }

  // setup the message
  hdr = genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, nl80211_id, 0,
      0, NL80211_CMD_JOIN_OCB, 0);
  if (hdr == NULL) {
    fprintf(stderr, "ERROR creating netlink message\n");
    status = 1;
    goto cleanup;
  }

  //add message attributes
  if_index_uint32 = if_index;
  if (nla_put(msg, NL80211_ATTR_IFINDEX, sizeof(uint32_t), &if_index_uint32)
      != 0) {
    fprintf(stderr, "ERROR setting ifindex attribute\n");
    status = 1;
    goto cleanup;
  }

  uint32_t freq;
  if (channel >= 1 && channel <= 11)
    freq = (2412 + 5*(channel-1));
  else {
    printf("Incorrect channel, using channel 1\n");
    freq = 2412;
  }
  if (nla_put(msg, NL80211_ATTR_WIPHY_FREQ, sizeof(uint32_t), &freq)
      != 0) {
    fprintf(stderr, "ERROR setting freq attribute\n");
    status = 1;
    goto cleanup;
  }

  uint32_t _width;
  if (width == 5)
    _width = NL80211_CHAN_WIDTH_5;
  else if (width == 10)
    _width = NL80211_CHAN_WIDTH_10;
  else {
    printf("Channel width must be 5 or 10, using 10\n");
    _width = NL80211_CHAN_WIDTH_10;
  }
  if (nla_put(msg, NL80211_ATTR_CHANNEL_WIDTH, sizeof(uint32_t), &_width)
      != 0) {
    fprintf(stderr, "ERROR setting width attribute\n");
    status = 1;
    goto cleanup;
  }

  //printf("Sending netlink message\n");
  //send the messge (this frees it)
  num_bytes = nl_send_auto_complete(nl_sock, msg);
  if (num_bytes < 0) {
    fprintf(stderr, "ERROR sending netlink message\n");
    status = 1;
    goto cleanup;
  }

  status = 1;
  nl_cb_err(cmd_cb, NL_CB_CUSTOM, error_handler, (void *)&status);
  nl_cb_set(cmd_cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler,
      (void *)&status);
  nl_cb_set(cmd_cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, (void *)&status);

  //block for message to return
  nl_recvmsgs(nl_sock, cmd_cb);

cleanup:

  if (sock_cb != NULL)
    nl_cb_put(sock_cb);

  if (cmd_cb != NULL)
    nl_cb_put(cmd_cb);

  if (msg != NULL)
    nlmsg_free(msg);

  if (nl_sock != NULL)
    nl_socket_free(nl_sock);

  return 0;

} /* main */

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
    void *arg)
{
  *(int *)arg = err->error;
  printf("nl error: %s\n", nl_geterror(err->error));
  return NL_STOP;
}


/* Called after all survey result messages */
static int finish_handler(struct nl_msg *msg, void *arg)
{
  *(int *)arg = 0;
  printf("nl finish\n");
  return NL_SKIP;
}


static int ack_handler(struct nl_msg *msg, void *arg)
{
  *(int *)arg = 0;
  //printf("nl ack\n");
  return NL_STOP;
}
