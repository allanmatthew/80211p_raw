# To cross compile:
#
# Set up as usual for bitbake:
# $ . setup-environment build
#
# In the build directory:
# $ bitbake meta-ide-support
# $ . tmp/environment-setup-cortexa9hf-vfp-neon-poky-linux-gnueabi
#
# Now a make in this directory should work.

VPATH = ./util

INCS = -I./util -I${OECORE_TARGET_SYSROOT}/usr/include/libnl3 -I./

CFLAGS += -Wall $(INCS)

LIBS = -lnl-3 -lnl-genl-3

SRCS_C = nl.c
SRCS_C += util/rawsock_util.c

OBJS = $(SRCS_C:.c=.o)

TX = tx
RX = rx
TX_REDIRECT = tx_redir
RX_REDIRECT = rx_redir

all: $(TX) $(RX) $(TX_REDIRECT) $(RX_REDIRECT)

$(TX): tx.o $(OBJS)
	$(CC) -o $(TX) $(OBJS) tx.o $(LIBS)

$(RX): rx.o $(OBJS)
	$(CC) -o $(RX) $(OBJS) rx.o $(LIBS)

$(TX_REDIRECT): tx_redirect.o $(OBJS)
	$(CC) -o $(RX) $(OBJS) tx_redirect.o $(LIBS)

$(RX_REDIRECT): rx_redirect.o $(OBJS)
	$(CC) -o $(RX) $(OBJS) rx_redirect.o $(LIBS)

clean:
	$(RM) util/*.o *.o *~ $(TX) $(RX) $(TX_REDIRECT) $(RX_REDIRECT)

.PHONY: clean
