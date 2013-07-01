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
	opt->initialised = 0;
}

static void usage(const char *name)
{
	fprintf(stderr, "Usage: %s [-46] [-l <length>] [-r <bps>] [-o <outfile>] <destination>\n", name);
}


int parse_options(struct options *opt, int argc, char **argv)
{
	int o;
	int val;
	FILE *output;
	struct addrinfo hints = {0};
	struct addrinfo *result;

	while((o = getopt(argc, argv, "46hl:o:r:")) != -1) {
		switch(o) {
		case '4':
			hints.ai_family = AF_INET;
			break;
		case '6':
			hints.ai_family = AF_INET6;
			break;
		case 'l':
			val = atoi(optarg);
			if(val < 1) {
				fprintf(stderr, "Invalid length: %d\n", val);
				return -1;
			}
			opt->run_length = val;
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
			opt->rate = val;
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
	memcpy(&opt->dest, result, sizeof(struct addrinfo));
	freeaddrinfo(result);
	opt->socket = socket(opt->dest.ai_family, SOCK_DGRAM, PF_UNSPEC);
	if(opt->socket == -1) {
		perror("Unable to open socket");
		return -1;
	}
	return 0;
}
