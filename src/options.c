/**
 * options.c
 *
 * Toke Høiland-Jørgensen
 * 2013-06-04
 */

#include <stdio.h>
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
	opt->initialised = 0;
}

static void usage(const char *name)
{
	fprintf(stderr, "Usage: %s [-l <length>] [-r <bps>] [-o <outfile>] <destination>\n", name);
}


int parse_options(struct options *opt, int argc, char **argv)
{
	int o;
	int val;
	FILE *output;
	struct addrinfo *result;

	while((o = getopt(argc, argv, "hl:o:r:")) != -1) {
		switch(o) {
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
	if(argc < 2) {
		usage(argv[0]);
		return -1;
	}
	if((val = getaddrinfo(argv[1], NULL, NULL, &result)) != 0) {
		fprintf(stderr, "Unable to lookup address '%s': %s\n", argv[1], gai_strerror(val));
		return -1;
	}
	memcpy(&opt->destination, result, sizeof(struct addrinfo));
	freeaddrinfo(result);
	return 0;
}
