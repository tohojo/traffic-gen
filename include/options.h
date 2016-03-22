/**
 * options.h
 *
 * Toke Høiland-Jørgensen
 * 2013-06-04
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


struct options {
	char initialised;
	char poisson_interval;
	char poisson_packets;
	int run_length;
	int pps;
	int pkt_size;
	int port_range;
	unsigned char tos;
	struct timeval start_time;
	FILE *output;
	struct addrinfo *dest;
	int socket;
};

int initialise_options(struct options *opt, int argc, char **argv);
void destroy_options(struct options *opt);

#endif
