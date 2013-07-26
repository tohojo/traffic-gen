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
#include "util.h"

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

static double exp_distrib(double mean)
{
	// Draw a random value and create an exponentially distributed value
	// from it. Calculated by -ln(r)/mean where r is a random number between
	// 0 and 1
	//
	// Ref: https://en.wikipedia.org/wiki/Exponential_distribution#Generating_exponential_variates
	double r;
	do {
		r = (double)rand() / (double) RAND_MAX;
	} while(r == 0.0); // log(0) is undefined
	return -log(r)/mean;
}

static unsigned int exp_wait(unsigned int pps)
{
        // Exponential wait is an exponentially distributed value with mean set
	// to achieve pps packets/sec.
	return (unsigned int) 1000000 * exp_distrib(pps);
}

static unsigned int scale_payload(unsigned int size, unsigned int overhead)
{
	double scale = exp_distrib(1.0);
	return min(max(0, (size * scale)-overhead), MAX_PAYLOAD);
}

static void schedule_next(unsigned int pps, char poisson, struct timeval *now, struct timeval *next)
{
	int delay;
	if(poisson)
		delay = exp_wait(pps);
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

	printf("Sending %d pps of size %d (%d+%d) bytes to %s for %d seconds (%s).\n",
		opt->pps, opt->pkt_size, payload, overhead,
		destaddr,
		opt->run_length,
		opt->poisson_interval ? "poisson" : "deterministic"
		);

	gettimeofday(&now, NULL);
	stop.tv_sec = now.tv_sec + opt->run_length;
	stop.tv_usec = now.tv_usec;
	srand(now.tv_sec ^ now.tv_usec);
	do {
		schedule_next(opt->pps, opt->poisson_interval, &now, &next);
		while(now.tv_sec < next.tv_sec || now.tv_usec < next.tv_usec) {
			if(next.tv_usec - now.tv_usec > USLEEP_THRESHOLD)
				usleep(USLEEP_THRESHOLD);
			gettimeofday(&now, NULL);
		}
		set_port(opt->dest, gen_port(opt->port_range));
		if(opt->poisson_packets)
			payload = scale_payload(opt->pkt_size, overhead);
		sendto(opt->socket, msg, payload, 0, opt->dest->ai_addr, opt->dest->ai_addrlen);
	} while(now.tv_sec < stop.tv_sec || now.tv_usec < stop.tv_usec);
}
