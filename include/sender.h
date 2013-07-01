/**
 * sender.h
 *
 * Toke Høiland-Jørgensen
 * 2013-07-01
 */

#ifndef SENDER_H
#define SENDER_H

#include "options.h"

#define DEST_PORT 10001

void send_loop(struct options *opt);

#endif
