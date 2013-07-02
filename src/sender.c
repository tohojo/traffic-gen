/**
 * sender.c
 *
 * Toke Høiland-Jørgensen
 * 2013-07-01
 */

#include <stdlib.h>
#include <math.h>
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

static unsigned short gen_port()
{
	return PORT_START + rand() % PORT_RANGE;
}

static unsigned int exp_distrib(unsigned int pps)
{
	// Draw a new delay from the exponential distribution with mean delay
	// set to achieve pps packets/sec. Calculated by -ln(r)/pps where r is a
	// random number between 0 and 1
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
	char msg[PAYLOAD] = {0};
	unsigned int pps = opt->rate / (OVERHEAD(opt->dest.ai_family) + PAYLOAD);

	gettimeofday(&now, NULL);
	stop.tv_sec = now.tv_sec + opt->run_length;
	stop.tv_usec = now.tv_usec;
	srand(now.tv_sec ^ now.tv_usec);
	do {
		schedule_next(pps, opt->poisson, &now, &next);
		while(now.tv_sec < next.tv_sec || now.tv_usec < next.tv_usec) {
			if(next.tv_usec - now.tv_usec > USLEEP_THRESHOLD)
				usleep(USLEEP_THRESHOLD);
			gettimeofday(&now, NULL);
		}
		set_port(&opt->dest, gen_port());
		sendto(opt->socket, msg, PAYLOAD, 0, opt->dest.ai_addr, opt->dest.ai_addrlen);
	} while(now.tv_sec < stop.tv_sec || now.tv_usec < stop.tv_usec);
}
