/**
 * sender.c
 *
 * Toke Høiland-Jørgensen
 * 2013-07-01
 */

#include <stdlib.h>
#include <math.h>
#include <sys/socket.h>
#include <netdb.h>
#include "sender.h"

static void set_port(struct addrinfo *addr, unsigned short port)
{
	if(addr->ai_family == AF_INET) {
		struct sockaddr_in *saddr = (struct sockaddr_in *)addr->ai_addr;
		saddr->sin_port = htons(port);
	} else if(addr->ai_family == AF_INET6) {
		struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)addr->ai_addr;
		saddr->sin6_port = htons(port);
	}
}

static unsigned short gen_port(range)
{
	return PORT_START + rand() % range;
}

static unsigned int exp_distrib(unsigned int pps)
{
	// Draw a new delay (in usec) from the exponential distribution with
	// mean delay set to achieve pps packets/sec. Calculated by -ln(r)/pps
	// where r is a random number between 0 and 1
	//
	// Ref: https://en.wikipedia.org/wiki/Exponential_distribution#Generating_exponential_variates
	double r;
	do {
		r = (double)rand() / (double) RAND_MAX;
	} while(r == 0.0);
	int d = (unsigned int) 1000000 * (-log(r)/((double)pps));
	return d;
}

static void schedule_next(unsigned int pps, char poisson, struct timeval *now, struct timeval *next)
{
	int delay;
	if(poisson)
		delay = exp_distrib(pps);
	else
		delay = 1000000/pps;
	next->tv_sec = now->tv_sec;
	next->tv_usec = now->tv_usec + delay;
	if(next->tv_usec > 1000000) {
		next->tv_sec += next->tv_usec / 1000000;
		next->tv_usec %= 1000000;
	}
}

void send_loop(struct options *opt)
{
	struct timeval now, next, stop;
	int ret;
	char msg[MAX_PAYLOAD] = {0};
	int overhead = OVERHEAD(opt->dest->ai_family);
	int payload = opt->pkt_size - overhead;
	char destaddr[INET6_ADDRSTRLEN];

	if(payload < 0) {
		fprintf(stderr, "Minimum packet size for selected host and address family is %d bytes\n",
			overhead);
		return;
	}

	if((ret = getnameinfo(opt->dest->ai_addr, opt->dest->ai_addrlen,
					destaddr, sizeof(destaddr),
					NULL, 0,
					NI_NUMERICHOST)) != 0) {
		fprintf(stderr, "Error creating address string: %s\n", gai_strerror(ret));
		return;
	}

	printf("Sending %d pps of size %d (%d+%d) bytes to %s for %d seconds.\n",
		opt->pps, opt->pkt_size, payload, overhead,
		destaddr,
		opt->run_length);

	gettimeofday(&now, NULL);
	stop.tv_sec = now.tv_sec + opt->run_length;
	stop.tv_usec = now.tv_usec;
	srand(now.tv_sec ^ now.tv_usec);
	do {
		schedule_next(opt->pps, opt->poisson, &now, &next);
		while(now.tv_sec < next.tv_sec || now.tv_usec < next.tv_usec) {
			if(next.tv_usec - now.tv_usec > USLEEP_THRESHOLD)
				usleep(USLEEP_THRESHOLD);
			gettimeofday(&now, NULL);
		}
		set_port(opt->dest, gen_port(opt->port_range));
		sendto(opt->socket, msg, payload, 0, opt->dest->ai_addr, opt->dest->ai_addrlen);
	} while(now.tv_sec < stop.tv_sec || now.tv_usec < stop.tv_usec);
}
