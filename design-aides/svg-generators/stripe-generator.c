/*
 *  moto-design stripe generator.
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

static const char program_name[] = "stripe-generator";

static void print_version(void)
{
	printf("%s (" PACKAGE_NAME ") " PACKAGE_VERSION "\n", program_name);
}

static void print_bugreport(void)
{
	fprintf(stderr, "Report bugs at " PACKAGE_BUGREPORT ".\n");
}

struct stripe_params {
	float top_angle;
	float bottom_angle;
	float lean_angle;
	unsigned int block_count;
	float block_height;
	float block_width;
	float block_multiplier;
	float gap_width;
	float gap_multiplier;
};

enum opt_value {opt_undef = 0, opt_yes, opt_no};

struct opts {
	struct stripe_params stripe_params;
	char *output_file;
	char *config_file;
	enum opt_value background;
	enum opt_value help;
	enum opt_value verbose;
	enum opt_value version;
};

static const struct stripe_params init_stripe_params = {
	.top_angle = HUGE_VALF,
	.bottom_angle = HUGE_VALF,
	.lean_angle = HUGE_VALF,
	.block_count = UINT_MAX,
	.block_height = HUGE_VALF,
	.block_width = HUGE_VALF,
	.block_multiplier = HUGE_VALF,
	.gap_width = HUGE_VALF,
	.gap_multiplier = HUGE_VALF,
};

static const struct stripe_params default_stripe_params = {
	.top_angle = 10.0,
	.bottom_angle = 5.0,
	.lean_angle = 100.0,
	.block_count = 8,
	.block_height = 150.0,
	.block_width = 200.0,
	.block_multiplier = 0.25,
	.gap_width = 45.0,
	.gap_multiplier = 0.25,
};

static void print_usage(const struct opts *opts)
{
	print_version();

	fprintf(stderr,
"%s - Generates SVG file of hannah stripes.\n"
"Usage: %s [flags]\n"
"Option flags:\n"
"  --top-angle        - angle. Default: '%f'.\n"
"  --bottom-angle     - angle. Default: '%f'.\n"
"  --lean-angle       - angle. Default: '%f'.\n"
"  --block-count      - block-count. Default: '%u'.\n"
"  --block-height     - height. Default: '%f'.\n"
"  --block-width      - width. Default: '%f'.\n"
"  --block-multiplier - multiplier. Default: '%f'.\n"
"  --gap-width        - width. Default: '%f'.\n"
"  --gap-multiplier   - multiplier. Default: '%f'.\n"

"  -o --output-file  - Output file. Default: '%s'.\n"
"  -f --config-file  - Config file. Default: '%s'.\n"
"  -b --background   - Generate image background. Default: '%s'.\n"
"  -h --help         - Show this help and exit.\n"
"  -v --verbose      - Verbose execution.\n"
"  -V --version      - Display the program version number.\n",
		program_name, program_name,

		opts->stripe_params.top_angle,
		opts->stripe_params.bottom_angle,
		opts->stripe_params.lean_angle,
		opts->stripe_params.block_count,
		opts->stripe_params.block_height,
		opts->stripe_params.block_width,
		opts->stripe_params.block_multiplier,
		opts->stripe_params.gap_width,
		opts->stripe_params.gap_multiplier,

		opts->output_file,
		opts->config_file,
		(opts->background ? "yes" : "no")
	);

	print_bugreport();
}

static int opts_parse(struct opts *opts, int argc, char *argv[])
{
	static const struct option long_options[] = {
		{"top-angle",        required_argument, NULL, '1'},
		{"bottom-angle",     required_argument, NULL, '2'},
		{"lean-angle",       required_argument, NULL, '3'},
		{"block-count",      required_argument, NULL, '4'},
		{"block-height",     required_argument, NULL, '5'},
		{"block-width",      required_argument, NULL, '6'},
		{"block-multiplier", required_argument, NULL, '7'},
		{"gap-width",        required_argument, NULL, '8'},
		{"gap-multiplier",   required_argument, NULL, '9'},

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
		.stripe_params = init_stripe_params,
		.output_file = "-",
		.config_file = NULL,
		.background = opt_no,
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
		// stripe
		case '1':
			opts->stripe_params.top_angle = to_float(optarg);
			if (opts->stripe_params.top_angle == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '2':
			opts->stripe_params.bottom_angle = to_float(optarg);
			if (opts->stripe_params.bottom_angle == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '3':
			opts->stripe_params.lean_angle = to_float(optarg);
			if (opts->stripe_params.lean_angle == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '4':
			opts->stripe_params.block_count = to_unsigned(optarg);
			if (opts->stripe_params.block_count == UINT_MAX) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '5':
			opts->stripe_params.block_height = to_float(optarg);
			if (opts->stripe_params.block_height == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '6':
			opts->stripe_params.block_width = to_float(optarg);
			if (opts->stripe_params.block_width == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '7':
			opts->stripe_params.block_multiplier = to_float(optarg);
			if (opts->stripe_params.block_multiplier == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '8':
			opts->stripe_params.gap_width = to_float(optarg);
			if (opts->stripe_params.gap_width == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '9':
			opts->stripe_params.gap_multiplier = to_float(optarg);
			if (opts->stripe_params.gap_multiplier == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case 'b':
			opts->background = opt_yes;
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
		case 'f': {
			size_t len;
			
			if (0) {
				debug("p:   %p = '%s'\n", optarg, optarg);
			}
			if (!optarg) {
				error("Missing required argument <config-file>.'\n");
				opts->help = opt_yes;
				return -1;
			}
			len = strlen(optarg) + 1;
			opts->config_file = mem_alloc(len);
			strcpy(opts->config_file, optarg);
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

static void write_background(FILE* out_stream,
	const struct svg_rect *background_rect, const char *fill_color)
{
	assert(is_hex_color(fill_color));

	svg_open_group(out_stream, "background");
	svg_write_rect(out_stream, "background", fill_color, NULL,
		background_rect);
	svg_close_group(out_stream);
}

struct block_params {
	char id[256];
	char fill[256];
	char stroke[256];
	struct point_c origin;
	struct point_c top_left;
	struct point_c top_right;
	struct point_c bottom_right;
;
};

static void write_block(FILE* out_stream, const struct block_params *block)
{
	svg_open_path(out_stream, block->id, block->fill, block->stroke);
	fprintf(out_stream, "   d=\"M %f,%f\n", block->origin.x, block->origin.y);
	fprintf(out_stream, "    L %f,%f\n", block->top_left.x, block->top_left.y);
	fprintf(out_stream, "    L %f,%f\n", block->top_right.x, block->top_right.y);
	fprintf(out_stream, "    L %f,%f\n", block->bottom_right.x, block->bottom_right.y);
	fprintf(out_stream, "    Z\"/>\n");
	svg_close_object(out_stream);
}

static void write_svg(FILE* out_stream,
	const struct stripe_params *stripe_params, bool background)
{
	unsigned int i;
	struct svg_rect background_rect;
	const float top_tan = tanf(deg_to_rad(stripe_params->top_angle));
	const float bottom_tan = tanf(deg_to_rad(stripe_params->bottom_angle));

	debug("bottom_tan = %f\n", bottom_tan);
	debug("top_tan    = %f\n", top_tan);
	
	background_rect.width = 1.6 * stripe_params->block_count *
		(stripe_params->block_width + stripe_params->gap_width);
	background_rect.height = 1.6 * stripe_params->block_height;

	background_rect.rx = 50.0;
	background_rect.x = -background_rect.rx / 2.0;
	background_rect.y = -background_rect.rx / 2.0;

	svg_open_svg(out_stream, &background_rect);

	if (background) {
		write_background(out_stream, &background_rect, "#0000ff");
	}

	svg_open_group(out_stream, "hannah_stripes");

	for (i = 0; i < stripe_params->block_count; i++) {
		struct block_params block;

		snprintf(block.id, sizeof(block.id), "block_%d", i);
		strcpy(block.fill, "009aff");
		strcpy(block.stroke, "");

		block.origin.x = i * (stripe_params->block_width + stripe_params->gap_width);
		block.origin.y = block.origin.x * bottom_tan;

		block.top_left.x = block.origin.x;
		block.top_left.y = block.origin.y + stripe_params->block_height;

		block.bottom_right.x = block.origin.x + stripe_params->block_width;
		block.bottom_right.y = block.bottom_right.x * bottom_tan;

		block.top_right.x = block.bottom_right.x;
		block.top_right.y =block.bottom_right.y + stripe_params->block_height;

		//debug("%u: (%u) = %u, %u\n", i, render_order[i], pos.column, pos.row);
		write_block(out_stream, &block);

	}

	svg_close_group(out_stream);
	svg_close_svg(out_stream);
}

struct config_cb_data {
	const char *config_file;
	struct stripe_params *stripe_params;
	struct palette* palette;
	struct color_data *color_data;
	unsigned color_counter;
};

static void config_cb(void *cb_data, const char *section, char *config_data)
{
	struct config_cb_data *cbd = cb_data;

	//debug("%s, '%s'\n", section, config_data);

	if (!strcmp(section, "[params]")) {
		char *name = strtok(config_data, "=");
		char *value = strtok(NULL, " \t");

		if (!name) {
			error("Bad config name, section %s: '%s'\n", section,
			      config_data);
			assert(0);
			exit(EXIT_FAILURE);
		}
		if (!value) {
			error("Bad config value, section %s: '%s'\n", section,
			      config_data);
			assert(0);
			exit(EXIT_FAILURE);
		}

		name = config_clean_data(name);
		value = config_clean_data(value);

		//debug("params: '%s', '%s'\n", name, value);

		if (cbd->stripe_params->top_angle ==
			init_stripe_params.top_angle &&
			!strcmp(name, "top_angle")) {
			cbd->stripe_params->top_angle =
				to_float(value);
		}
		if (cbd->stripe_params->bottom_angle ==
			init_stripe_params.bottom_angle &&
			!strcmp(name, "bottom_angle")) {
			cbd->stripe_params->bottom_angle =
				to_float(value);
		}
		if (cbd->stripe_params->lean_angle ==
			init_stripe_params.lean_angle &&
			!strcmp(name, "lean_angle")) {
			cbd->stripe_params->lean_angle =
				to_float(value);
		}
		if (cbd->stripe_params->block_count ==
			init_stripe_params.block_count &&
			!strcmp(name, "block_count")) {
			cbd->stripe_params->block_count =
				to_unsigned(value);
		}
		if (cbd->stripe_params->block_height ==
			init_stripe_params.block_height &&
			!strcmp(name, "block_height")) {
			cbd->stripe_params->block_height = to_float(value);
		}
		if (cbd->stripe_params->block_width ==
			init_stripe_params.block_width &&
			!strcmp(name, "block_width")) {
			cbd->stripe_params->block_width = to_float(value);
		}
		if (cbd->stripe_params->block_multiplier ==
			init_stripe_params.block_multiplier &&
			!strcmp(name, "block_multiplier")) {
			cbd->stripe_params->block_multiplier = to_float(value);
		}
		if (cbd->stripe_params->gap_width ==
			init_stripe_params.gap_width &&
			!strcmp(name, "gap_width")) {
			cbd->stripe_params->gap_width = to_float(value);
		}
		if (cbd->stripe_params->gap_multiplier ==
			init_stripe_params.gap_multiplier &&
			!strcmp(name, "gap_multiplier")) {
			cbd->stripe_params->gap_multiplier = to_float(value);
		}

		return;
	}

	if (!strcmp(section, "[palette]")) {
		char *weight = strtok(config_data, ",");
		char *value = strtok(NULL, " \t");

		if (!weight) {
			error("Bad config weight, section %s: '%s'\n", section,
			      config_data);
			assert(0);
			exit(EXIT_FAILURE);
		}
		if (!value) {
			error("Bad config value, section %s: '%s'\n", section,
			      config_data);
			assert(0);
			exit(EXIT_FAILURE);
		}

		weight = config_clean_data(weight);
		value = config_clean_data(value);

		//debug("palette: '%s', '%s'\n", weight, value);
		
		if (!value || !is_hex_color(value) ) {
			fprintf(stderr,
				"Bad config hex color value: '%s'\n",
				value);
			assert(0);
			exit(EXIT_FAILURE);
		}
		
		cbd->color_data = mem_realloc(cbd->color_data,
			sizeof(*cbd->color_data) * (cbd->color_counter + 1));
		cbd->color_data[cbd->color_counter].weight = to_unsigned(weight);
		memcpy(&cbd->color_data[cbd->color_counter].value, value,
			hex_color_len);
		cbd->color_counter++;

		return;
	}

	if (!strcmp(section, "ON_EXIT")) {
		if (cbd->color_data) {
			palette_fill(cbd->palette, cbd->color_data,
				cbd->color_counter);
			mem_free(cbd->color_data);
			cbd->color_data = NULL;
		} else {
			warn("No palette found in config file: '%s'\n",
				cbd->config_file);
		}
		
		return;
	}
	
	assert(0);
}

static void get_config_opts(struct opts *opts)
{
	static const char *sections[] = {
		"[params]",
	};
	struct config_cb_data cbd = {
		.config_file = opts->config_file,
		.stripe_params = &opts->stripe_params,
	};

	config_process_file(opts->config_file, config_cb, &cbd,
		sections, sizeof(sections)/sizeof(sections[0]));
}

int main(int argc, char *argv[])
{
	struct opts opts;
	FILE *out_stream;

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

	if (opts.config_file){
		get_config_opts(&opts);
	}

	if (opts.stripe_params.top_angle ==
		init_stripe_params.top_angle) {
		opts.stripe_params.top_angle =
			default_stripe_params.top_angle;
	}
	if (opts.stripe_params.bottom_angle ==
		init_stripe_params.bottom_angle) {
		opts.stripe_params.bottom_angle =
			default_stripe_params.bottom_angle;
	}
	if (opts.stripe_params.lean_angle ==
		init_stripe_params.lean_angle) {
		opts.stripe_params.lean_angle =
			default_stripe_params.lean_angle;
	}

	if (opts.stripe_params.block_count ==
		init_stripe_params.block_count) {
		opts.stripe_params.block_count =
			default_stripe_params.block_count;
	}
	if (opts.stripe_params.block_height ==
		init_stripe_params.block_height) {
		opts.stripe_params.block_height =
			default_stripe_params.block_height;
	}
	if (opts.stripe_params.block_width ==
		init_stripe_params.block_width) {
		opts.stripe_params.block_width =
			default_stripe_params.block_width;
	}
	if (opts.stripe_params.block_multiplier ==
		init_stripe_params.block_multiplier) {
		opts.stripe_params.block_multiplier =
			default_stripe_params.block_multiplier;
	}
	if (opts.stripe_params.gap_width ==
		init_stripe_params.gap_width) {
		opts.stripe_params.gap_width =
			default_stripe_params.gap_width;
	}
	if (opts.stripe_params.gap_multiplier ==
		init_stripe_params.gap_multiplier) {
		opts.stripe_params.gap_multiplier =
			default_stripe_params.gap_multiplier;
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

	if (opts.config_file){
		mem_free(opts.config_file);
		opts.config_file = NULL;
	}

	srand((unsigned int)time(NULL));

	write_svg(out_stream, &opts.stripe_params, opts.background);

	return EXIT_SUCCESS;
}

