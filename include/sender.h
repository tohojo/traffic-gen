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
#define PORT_RANGE 1000

#define USLEEP_THRESHOLD 10000

void send_loop(struct options *opt);

#endif
