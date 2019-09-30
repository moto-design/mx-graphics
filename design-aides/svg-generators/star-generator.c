/*
 *  moto-design star generator.
 */

/*
  PROJECT="${HOME}/projects/moto-design/mx-graphics/design-aides/svg-generators"
  (cd ${PROJECT} && ./bootstrap) && ${PROJECT}/configure --enable-debug
*/

#define _GNU_SOURCE
#define _ISOC99_SOURCE

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <linux/limits.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "util.h"

static const char program_name[] = "star-generator";

static void print_version(void)
{
	printf("%s (" PACKAGE_NAME ") " PACKAGE_VERSION "\n", program_name);
}

static void print_bugreport(void)
{
	fprintf(stderr, "Report bugs at " PACKAGE_BUGREPORT ".\n");
}

struct star_params {
	float radius;
};

enum opt_value {opt_undef = 0, opt_yes, opt_no};

struct opts {
	struct star_params star_params;
	char *output_file;
	enum opt_value help;
	enum opt_value verbose;
	enum opt_value version;
};

static const struct star_params init_star_params = {
	.radius = HUGE_VALF,
};

static const struct star_params default_star_params = {
	.radius = 10000.0 * 4.0 / 13.0 / 5.0 / 2.0,
};

static void print_usage(const struct opts *opts)
{
	print_version();

	fprintf(stderr,
"%s - Generates SVG file of 5 pointed star.\n"
"Usage: %s [flags]\n"
"Option flags:\n"
"  --radius          - Star radius. Default: '%f'.\n"
"  -o --output-file  - Output file. Default: '%s'.\n"
"  -h --help         - Show this help and exit.\n"
"  -v --verbose      - Verbose execution.\n"
"  -V --version      - Display the program version number.\n",
		program_name, program_name,
		opts->star_params.radius,
		opts->output_file
	);

	print_bugreport();
}

static int opts_parse(struct opts *opts, int argc, char *argv[])
{
	static const struct option long_options[] = {
		{"radius",     required_argument, NULL, '1'},

		{"output-file",    required_argument, NULL, 'o'},
		{"config-file",    required_argument, NULL, 'f'},
		{"background",     no_argument,       NULL, 'b'},
		{"help",           no_argument,       NULL, 'h'},
		{"verbose",        no_argument,       NULL, 'v'},
		{"version",        no_argument,       NULL, 'V'},
		{ NULL,            0,                 NULL, 0},
	};
	static const char short_options[] = "bo:f:hvV";

	*opts = (struct opts){
		.star_params = init_star_params,
		.output_file = "-",
		.help = opt_no,
		.verbose = opt_no,
		.version = opt_no,
	};

	if (0) {
		int i;

		debug("argc = %d\n", argc);
		for (i = 0; i < argc; i++) {
			debug("  %d: %p = '%s'\n", i, &argv[i],
				argv[i]);
		}
	}
	
	while (1) {
		int c = getopt_long(argc, argv, short_options, long_options,
			NULL);

		if (c == EOF)
			break;

		if (0) {
			debug("%c => '%s'\n", c, optarg);
		}

		switch (c) {
		// star
		case '1':
			opts->star_params.radius = to_float(optarg);
			if (opts->star_params.radius == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		// admin
		case 'o': {
			size_t len;
			
			if (0) {
				debug("p:   %p = '%s'\n", optarg, optarg);
			}
			if (!optarg) {
				error("Missing required argument <output-file>.'\n");
				opts->help = opt_yes;
				return -1;
			}
			len = strlen(optarg) + 1;
			opts->output_file = mem_alloc(len);
			strcpy(opts->output_file, optarg);
			break;
		}
		case 'h':
			opts->help = opt_yes;
			break;
		case 'v':
			opts->verbose = opt_yes;
			break;
		case 'V':
			opts->version = opt_yes;
			break;
		default:
			assert(0);
			opts->help = opt_yes;
			return -1;
		}
	}

	return optind != argc;
}

struct star {
	unsigned int node_count;
	struct node *nodes;
};

static void write_star(FILE* out_stream, const struct star_params *star_params)
{
	static const struct stroke stroke = {.color = "#0000ff", .width = 3};
	static const struct fill fill = {.color = "#ffdd00"};
	static const char star_id[] = "star_1";
	unsigned int node_count = 10;
	unsigned int node;

	svg_open_polygon(out_stream, star_id, fill.color, stroke.color, stroke.width);

	fprintf(out_stream, "     %f,%f\n", 0.0, 0.0);
	fprintf(out_stream, "     %f,%f\n", star_params->radius / 2, star_params->radius / 2);
	fprintf(out_stream, "     %f,%f\n", 0.0, star_params->radius);
	fprintf(out_stream, "     %f,%f\n", -star_params->radius / 2, star_params->radius / 2);
	fprintf(out_stream, "     %f,%f\n", 0.0, star_params->radius / 2);
	
	for (node = 0; node < node_count; node++) {
		struct point_c final = {0.0, 0.0};
		if (0)
			fprintf(out_stream, "     %f,%f\n", final.x, final.y);
	}

	svg_close_polygon(out_stream);
	svg_close_object(out_stream);
}

static void write_svg(FILE* out_stream, const struct star_params *star_params)
{
	struct svg_rect background_rect;

	background_rect.width = 3 * star_params->radius;
	background_rect.height = background_rect.width;

	background_rect.x = 0;
	background_rect.y = 0;

	svg_open_svg(out_stream, &background_rect);

	write_star(out_stream, star_params);

	svg_close_svg(out_stream);
}


int main(int argc, char *argv[])
{
	struct opts opts;
	FILE *out_stream;

	set_exit_on_error(true);

	if (opts_parse(&opts, argc, argv)) {
		print_usage(&opts);
		return EXIT_FAILURE;
	}

	if (opts.version == opt_yes) {
		print_version();
		return EXIT_SUCCESS;
	}

	if (opts.verbose == opt_yes) {
		set_verbose(true);
	}

	if (opts.star_params.radius == init_star_params.radius) {
		opts.star_params.radius = default_star_params.radius;
	}

	if (!strcmp(opts.output_file, "-")) {
		out_stream = stdout;
	} else {
		out_stream = fopen(opts.output_file, "w");
		if (!out_stream) {
			error("open <output-file> '%s' failed: %s\n",
				opts.output_file, strerror(errno));
			assert(0);
			return EXIT_FAILURE;
		}
	}

	if (opts.help == opt_yes) {
		print_usage(&opts);
		return EXIT_SUCCESS;
	}

	srand((unsigned int)time(NULL));

	write_svg(out_stream, &opts.star_params);

	return EXIT_SUCCESS;
}

