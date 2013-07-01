/**
 * sender.c
 *
 * Toke Høiland-Jørgensen
 * 2013-07-01
 */

#include <stdlib.h>
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

static void schedule_next(struct options *opt, struct timeval *now, struct timeval *next)
{
	next->tv_sec = now->tv_sec;
	next->tv_usec = now->tv_usec + 50000;
	if(next->tv_usec > 1000000) {
		next->tv_sec++;
		next->tv_usec %= 1000000;
	}
}

void send_loop(struct options *opt)
{
	struct timeval now, next, stop;
	char msg = {0};

	gettimeofday(&now, NULL);
	stop.tv_sec = now.tv_sec + opt->run_length;
	stop.tv_usec = now.tv_usec;
	srand(now.tv_sec ^ now.tv_usec);
	do {
		schedule_next(opt, &now, &next);
		while(now.tv_sec < next.tv_sec || now.tv_usec < next.tv_usec) {
			if(next.tv_usec - now.tv_usec > USLEEP_THRESHOLD)
				usleep(USLEEP_THRESHOLD);
			gettimeofday(&now, NULL);
		}
		set_port(&opt->dest, gen_port());
		sendto(opt->socket, &msg, 1, 0, opt->dest.ai_addr, opt->dest.ai_addrlen);
	} while(now.tv_sec < stop.tv_sec || now.tv_usec < stop.tv_usec);
}
