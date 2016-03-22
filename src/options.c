/**
 * options.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-04
 */

#include <string.h>
#include <stdlib.h>
#include "options.h"

int parse_options(struct options *opt, int argc, char **argv);


int initialise_options(struct options *opt, int argc, char **argv)
{
	opt->run_length = 60;
	opt->output = stdout;
	// default rate is 1Mbps
	opt->pps = 250;
	opt->pkt_size = 500;
	opt->port_range = 1000;
	opt->tos = 0x0;
	opt->poisson_interval = opt->poisson_packets = 1;
	gettimeofday(&opt->start_time, NULL);

	if(parse_options(opt, argc, argv) < 0)
		return -2;

	opt->initialised = 1;
	return 0;

}

void destroy_options(struct options *opt)
{
	if(!opt->initialised)
		return;
	close(opt->socket);
	freeaddrinfo(opt->dest);
	opt->initialised = 0;
}

static void usage(const char *name)
{
	fprintf(stderr, "Usage: %s [-46dD] [-f <flows>] [-l <length>] [-p <pps>] [-r <bps> | -s <pkt_size>] [-t <tos byte>] <destination>\n", name);
}


int parse_options(struct options *opt, int argc, char **argv)
{
	int o;
	int val;
	FILE *output;
	struct addrinfo hints = {0};
	struct addrinfo *result;

	while((o = getopt(argc, argv, "46dDf:hl:o:p:r:s:t:")) != -1) {
		switch(o) {
		case '4':
			hints.ai_family = AF_INET;
			break;
		case '6':
			hints.ai_family = AF_INET6;
			break;
		case 'D':
			opt->poisson_interval = 0;
			break;
		case 'd':
			opt->poisson_packets = 0;
			break;
		case 'f':
			// pps
			val = atoi(optarg);
			if(val < 1) {
				fprintf(stderr, "Invalid flows value: %d\n", val);
				return -1;
			}
			opt->port_range = val;
			break;
		case 'l':
			val = atoi(optarg);
			if(val < 1) {
				fprintf(stderr, "Invalid length: %d\n", val);
				return -1;
			}
			opt->run_length = val;
			break;
		case 'p':
			// pps
			val = atoi(optarg);
			if(val < 0) {
				fprintf(stderr, "Invalid pps value: %d\n", val);
				return -1;
			}
			opt->pps = val;
			break;
		case 'o':
			if(opt->output != stdout) {
				fprintf(stderr, "Output file already set.\n");
				return -1;
			}
			if(strcmp(optarg, "-") != 0) {
				output = fopen(optarg, "w");
				if(output == NULL) {
					perror("Unable to open output file");
					return -1;
				}
				opt->output = output;
			}
			break;
		case 'r':
			val = atoi(optarg);
			if(val < 1) {
				fprintf(stderr, "Invalid rate: %d\n", val);
				return -1;
			}
			opt->pps = (val/8)/opt->pkt_size;
			break;
		case 's':
			// packet size
			val = atoi(optarg);
			if(val < 1 || val > 1514) {
				fprintf(stderr, "Invalid packet size: %d\n", val);
				return -1;
			}
			opt->pkt_size = val;
			break;
		case 't':
			// TOS byte
			val = strtoul(optarg, NULL, 0);
			if(val < 0 || val > 255) {
				fprintf(stderr, "Invalid TOS byte value: 0x%x\n", val);
				return -1;
			}
			opt->tos = val;
			break;
		case 'h':
		default:
			usage(argv[0]);
			return -1;
			break;
		}
	}
	if(optind >= argc) {
		usage(argv[0]);
		return -1;
	}
	if((val = getaddrinfo(argv[optind], NULL, &hints, &result)) != 0) {
		fprintf(stderr, "Unable to lookup address '%s': %s\n", argv[optind], gai_strerror(val));
		return -1;
	}
	opt->dest = result;
	opt->socket = socket(opt->dest->ai_family, SOCK_DGRAM, PF_UNSPEC);
	if(opt->socket == -1) {
		perror("Unable to open socket");
		return -1;
	}
	if(opt->tos > 0 &&
		setsockopt(opt->socket, IPPROTO_IP, IP_TOS, &opt->tos, sizeof(opt->tos))) {
		perror("Unable to set TOS value");
		return -1;
	}
	return 0;
}
