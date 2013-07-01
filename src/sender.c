/**
 * sender.c
 *
 * Toke Høiland-Jørgensen
 * 2013-07-01
 */

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

void send_loop(struct options *opt)
{
	struct timeval now, next;
	char msg = {0};
	do {
		gettimeofday(&now, NULL);
		set_port(&opt->dest, DEST_PORT);
		sendto(opt->socket, &msg, 1, 0, opt->dest.ai_addr, opt->dest.ai_addrlen);
	} while(now.tv_sec < opt->start_time.tv_sec + opt->run_length);
}
