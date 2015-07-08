#ifndef __NL_H__
#define __NL_H__

#include <stdio.h>
#include <netlink/netlink.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include "nl80211_copy.h"

#define NL80211_CHAN_WIDTH_5 (NL80211_CHAN_WIDTH_160+1)
#define NL80211_CHAN_WIDTH_10 (NL80211_CHAN_WIDTH_5+1)

int nl_leave_ocb(void);
int nl_join_ocb(int channel, int width);
int nl_set_channel(int channel, int width);
#endif
