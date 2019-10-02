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
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "svg-utils.h"

static const char program_name[] = "stripe-generator";

static void print_version(void)
{
	printf("%s (" PACKAGE_NAME ") " PACKAGE_VERSION "\n", program_name);
}

static void print_bugreport(void)
{
	fprintf(stderr, "Report bugs at " PACKAGE_BUGREPORT ".\n");
}

struct edge_params {
	float start;
	float end;
};

struct stripe_params {
	unsigned int block_count;
	float top_angle;
	float bottom_angle;
	float lean_angle;
	float block_height;
	float block_width;
	float block_multiplier;
	float gap_width;
	float gap_multiplier;
	struct edge_params first_edge;
	struct edge_params second_edge;
	struct edge_params third_edge;
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
	.block_count = UINT_MAX,
	.top_angle = HUGE_VALF,
	.bottom_angle = HUGE_VALF,
	.lean_angle = HUGE_VALF,
	.block_height = HUGE_VALF,
	.block_width = HUGE_VALF,
	.gap_width = HUGE_VALF,
	.block_multiplier = HUGE_VALF,
	.gap_multiplier = HUGE_VALF,
	.first_edge = {
		.start = HUGE_VALF,
		.end = HUGE_VALF,
	},
	.second_edge = {
		.start = HUGE_VALF,
		.end = HUGE_VALF,
	},
	.third_edge = {
		.start = HUGE_VALF,
		.end = HUGE_VALF,
	},
};

static const struct stripe_params default_stripe_params = {
	.block_count = 10,
	.top_angle = 172.0,
	.bottom_angle = -1.0,
	.lean_angle = 60,
	.block_height = 150.0,
	.block_width = 210.0,
	.gap_width = 19.0,
	.block_multiplier = 0.85,
	.gap_multiplier = 0.91,
	.first_edge = {
		.start = 1.1,
		.end = 1.2,
	},
	.second_edge = {
		.start = 2.1,
		.end = 2.1,
	},
	.third_edge = {
		.start = 3.1,
		.end = 3.2,
	},
};

static void print_usage(const struct opts *opts)
{
	print_version();

	fprintf(stderr,
"%s - Generates SVG file of hannah stripes.\n"
"Usage: %s [flags]\n"
"Option flags:\n"
"  --block-count       - block-count. Default: '%u'.\n"
"  --top-angle         - angle. Default: '%f'.\n"
"  --bottom-angle      - angle. Default: '%f'.\n"
"  --lean-angle        - angle. Default: '%f'.\n"
"  --block-height      - height. Default: '%f'.\n"
"  --block-width       - width. Default: '%f'.\n"
"  --gap-width         - width. Default: '%f'.\n"
"  --block-multiplier  - multiplier. Default: '%f'.\n"
"  --gap-multiplier    - multiplier. Default: '%f'.\n"
"  --first-edge-start  - edge width. Default: '%f'.\n"
"  --first-edge-end    - edge width. Default: '%f'.\n"
"  --second-edge-start - edge width. Default: '%f'.\n"
"  --second-edge-end   - edge width. Default: '%f'.\n"
"  --third-edge-start  - edge width. Default: '%f'.\n"
"  --third-edge-end    - edge width. Default: '%f'.\n"

"  -o --output-file   - Output file. Default: '%s'.\n"
"  -f --config-file   - Config file. Default: '%s'.\n"
"  -b --background    - Generate image background. Default: '%s'.\n"
"  -h --help          - Show this help and exit.\n"
"  -v --verbose       - Verbose execution.\n"
"  -V --version       - Display the program version number.\n",
		program_name, program_name,

		opts->stripe_params.block_count,
		opts->stripe_params.top_angle,
		opts->stripe_params.bottom_angle,
		opts->stripe_params.lean_angle,
		opts->stripe_params.block_height,
		opts->stripe_params.block_width,
		opts->stripe_params.gap_width,
		opts->stripe_params.block_multiplier,
		opts->stripe_params.gap_multiplier,
		opts->stripe_params.first_edge.start,
		opts->stripe_params.first_edge.end,
		opts->stripe_params.second_edge.start,
		opts->stripe_params.second_edge.end,
		opts->stripe_params.third_edge.start,
		opts->stripe_params.third_edge.end,

		opts->output_file,
		opts->config_file,
		(opts->background ? "yes" : "no")
	);

	print_bugreport();
}

static int opts_parse(struct opts *opts, int argc, char *argv[])
{
	static const struct option long_options[] = {
		{"block-count",      required_argument, NULL, 'A'},
		{"top-angle",        required_argument, NULL, 'B'},
		{"bottom-angle",     required_argument, NULL, 'C'},
		{"lean-angle",       required_argument, NULL, 'D'},
		{"block-height",     required_argument, NULL, 'E'},
		{"block-width",      required_argument, NULL, 'F'},
		{"gap-width",        required_argument, NULL, 'G'},
		{"block-multiplier", required_argument, NULL, 'H'},
		{"gap-multiplier",   required_argument, NULL, 'I'},

		{"first-edge-start",  required_argument, NULL, 'J'},
		{"first-edge-end",    required_argument, NULL, 'K'},
		{"second-edge-start", required_argument, NULL, 'L'},
		{"second-edge-end",   required_argument, NULL, 'M'},
		{"third-edge-start",  required_argument, NULL, 'N'},
		{"third-edge-end",    required_argument, NULL, 'O'},

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
		case 'A':
			opts_set_value(opts->stripe_params.block_count, to_unsigned(optarg), UINT_MAX);
			break;
		case 'B':
			opts_set_value(opts->stripe_params.top_angle, to_float(optarg), HUGE_VALF);
			break;
		case 'C':
			opts_set_value(opts->stripe_params.bottom_angle, to_float(optarg), HUGE_VALF);
			break;
		case 'D':
			opts_set_value(opts->stripe_params.lean_angle, to_float(optarg), HUGE_VALF);
			break;
		case 'E':
			opts_set_value(opts->stripe_params.block_height, to_float(optarg), HUGE_VALF);
			break;
		case 'F':
			opts_set_value(opts->stripe_params.block_width, to_float(optarg), HUGE_VALF);
			break;
		case 'G':
			opts_set_value(opts->stripe_params.gap_width, to_float(optarg), HUGE_VALF);
			break;
		case 'H':
			opts_set_value(opts->stripe_params.block_multiplier, to_float(optarg), HUGE_VALF);
			break;
		case 'I':
			opts_set_value(opts->stripe_params.gap_multiplier, to_float(optarg), HUGE_VALF);
			break;

		case 'J':
			opts_set_value(opts->stripe_params.first_edge.start, to_float(optarg), HUGE_VALF);
			break;
		case 'K':
			opts_set_value(opts->stripe_params.first_edge.end, to_float(optarg), HUGE_VALF);
			break;
		case 'L':
			opts_set_value(opts->stripe_params.second_edge.start, to_float(optarg), HUGE_VALF);
			break;
		case 'M':
			opts_set_value(opts->stripe_params.second_edge.end, to_float(optarg), HUGE_VALF);
			break;
		case 'N':
			opts_set_value(opts->stripe_params.third_edge.start, to_float(optarg), HUGE_VALF);
			break;
		case 'O':
			opts_set_value(opts->stripe_params.third_edge.end, to_float(optarg), HUGE_VALF);
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
	svg_write_rect(out_stream, "background", fill_color, NULL, 0,
		background_rect);
	svg_close_group(out_stream);
}

struct block_params {
	char id[256];
	struct fill fill;
	struct stroke stroke;
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

	svg_open_path(out_stream, block->id, block->fill.color, block->stroke.color, block->stroke.width);
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

struct start_points {
	struct point_c top;
	struct point_c bottom;
};

static struct block_params* fill_block_array(
	const struct stripe_params *stripe_params,
	const struct start_points *start)
{
	unsigned int i;
	float gap_width;
	float block_width;
	struct block_params* block_array;
	const struct stripe_factors stripe_factors =
		get_stripe_factors(stripe_params);

	block_width = stripe_params->block_width;
	gap_width = stripe_params->gap_width;

	block_array = mem_alloc((stripe_params->block_count + 1) * sizeof(block_array[0]));
	block_array[0].bottom_right = start->bottom;
	block_array[0].top_right = start->top;

	for (i = 1; i < stripe_params->block_count + 1; i++) {
		debug("width = (%f,%f)\n", block_width, gap_width);

		snprintf(block_array[i].id, sizeof(block_array[i].id),
			"block_%d", i);
		//strcpy(block_array[i].fill.color, "#000099");
		fill_set(&block_array[i].fill, "#deff00");
		stroke_set(&block_array[i].stroke, NULL, 0);

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

	return block_array;
}

struct edges {
	struct block_params first;
	struct block_params second;
	struct block_params third;
};

static struct edges get_edges(const struct stripe_params *stripe_params,
	const struct block_params* block_array)
{
	struct edges edges;

#pragma message "...do from here"

	(void)stripe_params;

	edges.first.bottom_left = block_array[0].bottom_right;
	//block_array[0].top_right;

	return edges;
}

static void write_svg(FILE* out_stream,
	const struct stripe_params *stripe_params, bool background)
{
	const float tan_top = tanf(deg_to_rad(stripe_params->top_angle));
	const float tan_bottom = tanf(deg_to_rad(stripe_params->bottom_angle));
	const float tan_lean = tanf(deg_to_rad(stripe_params->lean_angle));
	const float lean = stripe_params->block_height / tan_lean;
	unsigned int i;
	struct svg_rect background_rect;
	struct start_points start;
	struct block_params* block_array;
	struct edges edges;

	(void)tan_bottom;

	debug("tan_top    = %f\n", tan_top);
	debug("tan_bottom = %f\n", tan_bottom);
	debug("lean       = %f (%f)\n", lean, 1.0 / tan_lean);

	start.bottom.x = 0;
	start.bottom.y = 0;
	start.top.x = start.bottom.x + stripe_params->block_height / tan_lean;
	start.top.y = start.bottom.y + stripe_params->block_height;

	debug("start.bottom = (%f,%f)\n", start.bottom.x, start.bottom.y);
	debug("start.top = (%f,%f)\n", start.top.x, start.top.y);

	block_array = fill_block_array(stripe_params, &start);

	background_rect.rx = 50;
	background_rect.x = min_f(start.bottom.x, start.top.x) - background_rect.rx;
	background_rect.y = start.bottom.y + background_rect.rx;
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

	edges = get_edges(stripe_params, block_array);

	write_block(out_stream, &edges.first);

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

		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., block_count, to_unsigned(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., top_angle, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., bottom_angle, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., lean_angle, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., block_height, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., block_width, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., gap_width, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., block_multiplier, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., gap_multiplier, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., first_edge.start, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., first_edge.end, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., second_edge.start, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., second_edge.end, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., third_edge.start, to_float(value));
		cbd_set_value(cbd->stripe_params, init_stripe_params, name, stripe., third_edge.end, to_float(value));

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

	if (opts.config_file){
		get_config_opts(&opts);
	}

	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, block_count);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, top_angle);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, bottom_angle);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, lean_angle);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, block_height);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, block_width);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, gap_width);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, block_multiplier);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, gap_multiplier);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, first_edge.start);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, first_edge.end);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, second_edge.start);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, second_edge.end);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, third_edge.start);
	opts_set_default(opts.stripe_params, init_stripe_params, default_stripe_params, third_edge.end);

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

