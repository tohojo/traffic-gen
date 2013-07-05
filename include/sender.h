/**
 * sender.h
 *
 * Toke Høiland-Jørgensen
 * 2013-07-01
 */

#ifndef SENDER_H
#define SENDER_H

#include "options.h"

#define PORT_START 10000

#define USLEEP_THRESHOLD 10000

#define OVERHEAD_ETHERNET 14
#define OVERHEAD_IP 20
#define OVERHEAD_IP6 40
#define OVERHEAD_UDP 8

#define OVERHEAD(family) (OVERHEAD_ETHERNET + ((family == AF_INET) ? OVERHEAD_IP : OVERHEAD_IP6) + OVERHEAD_UDP)
#define PAYLOAD(family, size) (size-OVERHEAD(family))
#define MAX_PAYLOAD (1514 - OVERHEAD(AF_INET6))

void send_loop(struct options *opt);

#endif
