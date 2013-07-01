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

static unsigned int calc_delay(int family, int rate)
{
	int size = OVERHEAD(family) + PAYLOAD;
	return 1000000 / (rate/size);
}

static unsigned int exp_distrib(unsigned int delay)
{
	// Draw a new delay from the exponential distribution with mean delay
	// set to the parameter. Calculated by -ln(r)/delay where r is a random
	// number between 0 and 1
	double r = (double)rand() / (double) RAND_MAX;
	if(r == 0.0) return 0; // FIXME not correct; what do to?
	int d = (unsigned int) (-log(r)/((double)delay/2000000.0));
	return d;
}

static void schedule_next(unsigned int delay, char poisson, struct timeval *now, struct timeval *next)
{
	if(poisson)
		delay = exp_distrib(delay);
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
	unsigned int delay = calc_delay(opt->dest.ai_family, opt->rate);

	gettimeofday(&now, NULL);
	stop.tv_sec = now.tv_sec + opt->run_length;
	stop.tv_usec = now.tv_usec;
	srand(now.tv_sec ^ now.tv_usec);
	printf("Size: %d\n", OVERHEAD(opt->dest.ai_family) + PAYLOAD);
	do {
		schedule_next(delay, opt->poisson, &now, &next);
		while(now.tv_sec < next.tv_sec || now.tv_usec < next.tv_usec) {
			if(next.tv_usec - now.tv_usec > USLEEP_THRESHOLD)
				usleep(USLEEP_THRESHOLD);
			gettimeofday(&now, NULL);
		}
		set_port(&opt->dest, gen_port());
		sendto(opt->socket, msg, PAYLOAD, 0, opt->dest.ai_addr, opt->dest.ai_addrlen);
	} while(now.tv_sec < stop.tv_sec || now.tv_usec < stop.tv_usec);
}
