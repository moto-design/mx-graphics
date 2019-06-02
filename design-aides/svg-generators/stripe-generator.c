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
	.top_angle = 172.0,
	.bottom_angle = -1.0,
	.lean_angle = 60,
	.block_count = 10,
	.block_height = 150.0,
	.block_width = 210.0,
	.block_multiplier = 0.85,
	.gap_width = 19.0,
	.gap_multiplier = 0.91,
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
	struct point_c bottom_left;
	struct point_c top_left;
	struct point_c top_right;
	struct point_c bottom_right;
};

struct line_factors {
	float a;
	float x;
	float y;
};

struct stripe_factors {
	struct line_factors top;
	struct line_factors bottom;
};

static struct stripe_factors get_stripe_factors(const struct stripe_params *sp)
{
	struct stripe_factors sf;
	const float lean_rad = deg_to_rad(sp->lean_angle);

	sf.top.a = sinf(deg_to_rad(sp->top_angle))
		/ sinf(deg_to_rad(sp->lean_angle - sp->top_angle));
	sf.top.x = cosf(lean_rad);
	sf.top.y = sinf(lean_rad);

	sf.bottom.a = sinf(deg_to_rad(sp->bottom_angle))
		/ sinf(deg_to_rad(sp->lean_angle - sp->bottom_angle));
	sf.bottom.x = sf.top.x;
	sf.bottom.y = sf.top.y;

	return sf;
}

static void write_block(FILE* out_stream, const struct block_params *block)
{
	debug("%s\n", block->id);
	debug(" BL %f,%f\n", block->bottom_left.x, block->bottom_left.y);
	debug(" BR %f,%f\n", block->bottom_right.x, block->bottom_right.y);
	debug(" TL %f,%f\n", block->top_left.x, block->top_left.y);
	debug(" TR %f,%f\n", block->top_right.x, block->top_right.y);

	svg_open_path(out_stream, block->id, block->fill, block->stroke);
	fprintf(out_stream, "   d=\"M %f,%f\n", block->bottom_left.x, block->bottom_left.y);
	fprintf(out_stream, "    L %f,%f\n", block->top_left.x, block->top_left.y);
	fprintf(out_stream, "    L %f,%f\n", block->top_right.x, block->top_right.y);
	fprintf(out_stream, "    L %f,%f\n", block->bottom_right.x, block->bottom_right.y);
	fprintf(out_stream, "    Z\"\n");
	svg_close_object(out_stream);
}

static struct point_c next_point(const struct point_c* start,
	float width, const struct line_factors *lf)
{
	struct point_c next;
	float a;

	a = width * lf->a;

	next.x = start->x + width + a * lf->x;
	next.y = start->y + a * lf->y;

	return next;
}

static void write_svg(FILE* out_stream,
	const struct stripe_params *stripe_params, bool background)
{
	const float tan_top = tanf(deg_to_rad(stripe_params->top_angle));
	const float tan_bottom = tanf(deg_to_rad(stripe_params->bottom_angle));
	const float tan_lean = tanf(deg_to_rad(stripe_params->lean_angle));
	const float lean = stripe_params->block_height / tan_lean;
	const struct stripe_factors stripe_factors =
		get_stripe_factors(stripe_params);

	unsigned int i;
	struct svg_rect background_rect;
	struct point_c start_bottom;
	struct point_c start_top;
	struct block_params* block_array;
	float block_width;
	float gap_width;

	debug("tan_top    = %f\n", tan_top);
	debug("tan_bottom = %f\n", tan_bottom);
	debug("lean       = %f (%f)\n", lean, 1.0 / tan_lean);

	start_bottom.x = 0;
	start_bottom.y = 0;
	debug("start_bottom = (%f,%f)\n", start_bottom.x, start_bottom.y);

	start_top.x = start_bottom.x + stripe_params->block_height / tan_lean;
	start_top.y = start_bottom.y + stripe_params->block_height;
	debug("start_top = (%f,%f)\n", start_top.x, start_top.y);

	background_rect.rx = 50;
	background_rect.x = min_f(start_bottom.x, start_top.x) - background_rect.rx;
	background_rect.y = start_bottom.y + background_rect.rx;
	debug("background x,y = (%f,%f)\n", background_rect.x, background_rect.y);
	
	background_rect.width = 2.0 * background_rect.rx - lean
		+ stripe_params->block_count *
			(stripe_params->block_width + stripe_params->gap_width)
		- stripe_params->gap_width;
	background_rect.height = 2.0 * background_rect.rx
		+ stripe_params->block_height
		+ background_rect.width * tan_top;

	debug("background w,h = (%f,%f)\n", background_rect.width, background_rect.height);

	svg_open_svg(out_stream, &background_rect);

	if (0 && background) {
		write_background(out_stream, &background_rect, "#eeeeee");
	}

	svg_open_group(out_stream, "hannah_stripes");


	block_width = stripe_params->block_width;
	gap_width = stripe_params->gap_width;

	block_array = mem_alloc((stripe_params->block_count + 1) * sizeof(block_array[0]));
	block_array[0].bottom_right = start_bottom;
	block_array[0].top_right = start_top;

	for (i = 1; i < stripe_params->block_count + 1; i++) {
		debug("width = (%f,%f)\n", block_width, gap_width);

		snprintf(block_array[i].id, sizeof(block_array[i].id),
			"block_%d", i);
		strcpy(block_array[i].fill, "#000099");
		strcpy(block_array[i].stroke, "");

		block_array[i].bottom_left = next_point(
			&block_array[i - 1].bottom_right, gap_width,
			&stripe_factors.bottom);
		block_array[i].bottom_right = next_point(
			&block_array[i].bottom_left, block_width,
			&stripe_factors.bottom);

		block_array[i].top_left = next_point(&block_array[i - 1].top_right,
			gap_width, &stripe_factors.top);
		block_array[i].top_right = next_point(&block_array[i].top_left,
			block_width, &stripe_factors.top);

		block_width *= stripe_params->block_multiplier;
		gap_width *= stripe_params->gap_multiplier;
	}

	for (i = 1; i < stripe_params->block_count + 1; i++) {
		write_block(out_stream, &block_array[i]);
	}

	mem_free(block_array);

	svg_close_group(out_stream);
	svg_close_svg(out_stream);
}

struct config_cb_data {
	const char *config_file;
	struct stripe_params *stripe_params;
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

		debug("params: '%s', '%s'\n", name, value);

		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe_, top_angle, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe_, bottom_angle, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe_, lean_angle, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe_, block_count, to_unsigned(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe_, block_height, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe_, block_width, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe_, block_multiplier, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe_, gap_width, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe_, gap_multiplier, to_float(value));

		return;
	}

	if (!strcmp(section, "ON_EXIT")) {
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

	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, top_angle);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, bottom_angle);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, lean_angle);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, block_count);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, block_height);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, block_width);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, block_multiplier);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, gap_width);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, gap_multiplier);

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

