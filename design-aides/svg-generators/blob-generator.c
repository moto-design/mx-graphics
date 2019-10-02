/*
 *  moto-design blob generator.
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

static const char program_name[] = "blob-generator";

static void print_version(void)
{
	printf("%s (" PACKAGE_NAME ") " PACKAGE_VERSION "\n", program_name);
}

static void print_bugreport(void)
{
	fprintf(stderr, "Report bugs at " PACKAGE_BUGREPORT ".\n");
}

struct blob_params {
	unsigned int node_count_min;
	unsigned int node_count_max;
	float radius_min;
	float radius_max;
	float sector_min;
};

struct grid_params {
	unsigned int columns;
	unsigned int rows;
	float width;
	float wiggle;
};

enum opt_value {opt_undef = 0, opt_yes, opt_no};

struct opts {
	struct blob_params blob_params;
	struct grid_params grid_params;
	char *output_file;
	char *config_file;
	enum opt_value background;
	enum opt_value help;
	enum opt_value verbose;
	enum opt_value version;
};

static const struct blob_params init_blob_params = {
	.node_count_min = UINT_MAX,
	.node_count_max = UINT_MAX,
	.radius_min = HUGE_VALF,
	.radius_max = HUGE_VALF,
	.sector_min = HUGE_VALF,
};

static const struct grid_params init_grid_params = {
	.columns = UINT_MAX,
	.rows = UINT_MAX,
	.width = HUGE_VALF,
	.wiggle = HUGE_VALF,
};

static const struct blob_params default_blob_params = {
	.node_count_min = 8U,
	.node_count_max = 16U,
	.radius_min = 18.0,
	.radius_max = 70.0,
	.sector_min = 15.0,
};

static const struct grid_params default_grid_params = {
	.columns = 15U,
	.rows = 15U,
};

static void print_usage(const struct opts *opts)
{
	print_version();

	fprintf(stderr,
"%s - Generates SVG file of camouflage blobs.\n"
"Usage: %s [flags]\n"
"Option flags:\n"
"  --node-count-min - Blob minimum node count. Default: '%u'.\n"
"  --node-count-max - Blob maximum node count. Default: '%u'.\n"
"  --radius-min     - Blob minimum node radius. Default: '%f'.\n"
"  --radius-max     - Blob maximum node radius. Default: '%f'.\n"
"  --sector_min     - Blob minimum node sector angle. Default: '%f'.\n"

"  --grid-columns   - Output width. Default: '%u'.\n"
"  --grid-rows      - Output length. Default: '%u'.\n"
"  --grid-width     - Output grid width. Default: '%f'.\n"
"  --grid-wiggle    - Output grid wiggle. Default: '%f'.\n"

"  -o --output-file  - Output file. Default: '%s'.\n"
"  -f --config-file  - Config file. Default: '%s'.\n"
"  -b --background   - Generate image background. Default: '%s'.\n"
"  -h --help         - Show this help and exit.\n"
"  -v --verbose      - Verbose execution.\n"
"  -V --version      - Display the program version number.\n",
		program_name, program_name,

		opts->blob_params.node_count_min,
		opts->blob_params.node_count_max,
		opts->blob_params.radius_min,
		opts->blob_params.radius_max,
		opts->blob_params.sector_min,

		opts->grid_params.columns,
		opts->grid_params.rows,
		opts->grid_params.width,
		opts->grid_params.wiggle,

		opts->output_file,
		opts->config_file,
		(opts->background ? "yes" : "no")
	);

	print_bugreport();
}

static int opts_parse(struct opts *opts, int argc, char *argv[])
{
	static const struct option long_options[] = {
		{"node-count-min", required_argument, NULL, '1'},
		{"node-count-max", required_argument, NULL, '2'},
		{"radius-min",     required_argument, NULL, '3'},
		{"radius-max",     required_argument, NULL, '4'},
		{"sector_min",     required_argument, NULL, '5'},

		{"grid-columns",   required_argument, NULL, '6'},
		{"grid-rows",      required_argument, NULL, '7'},
		{"grid-width",     required_argument, NULL, '8'},
		{"grid-wiggle",    required_argument, NULL, '9'},

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
		.blob_params = init_blob_params,
		.grid_params = init_grid_params,
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
		// blob
		case '1':
			opts->blob_params.node_count_min = to_unsigned(optarg);
			if (opts->blob_params.node_count_min == UINT_MAX) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '2':
			opts->blob_params.node_count_max = to_unsigned(optarg);
			if (opts->blob_params.node_count_max == UINT_MAX) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '3':
			opts->blob_params.radius_min = to_float(optarg);
			if (opts->blob_params.radius_min == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '4':
			opts->blob_params.radius_max = to_float(optarg);
			if (opts->blob_params.radius_max == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '5':
			opts->blob_params.sector_min = to_float(optarg);
			if (opts->blob_params.sector_min == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		// grid
		case '6':
			opts->grid_params.columns = to_unsigned(optarg);
			if (opts->grid_params.columns == UINT_MAX) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '7':
			opts->grid_params.rows = to_unsigned(optarg);
			if (opts->grid_params.rows == UINT_MAX) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '8':
			opts->grid_params.width = to_float(optarg);
			if (opts->grid_params.width == HUGE_VALF) {
				opts->help = opt_yes;
				return -1;
			}
			break;
		case '9':
			opts->grid_params.wiggle = to_float(optarg);
			if (opts->grid_params.wiggle == HUGE_VALF) {
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

struct blob {
	unsigned int node_count;
	struct node *nodes;
};

struct grid_position {
	unsigned int row;
	unsigned int column;
	unsigned int number;
};

static void write_blob(FILE* out_stream, const struct grid_params *grid_params,
	const struct blob_params *blob_params, const char *color,
	const struct grid_position *pos)
{
	char blob_id[256];
	unsigned int node_count;
	unsigned int node;
	struct point_p point_p;
	struct point_c blob_offset;

	snprintf(blob_id, sizeof(blob_id), "blob_%d", pos->number);
	node_count = random_int(blob_params->node_count_min,
		blob_params->node_count_max);

	blob_offset.x = pos->column * grid_params->width
		+ random_float(0, grid_params->wiggle);
	blob_offset.y = pos->row * grid_params->width +
		random_float(0, grid_params->wiggle);

	log("%s: %u nodes at {%u,%u} => {%f,%f}\n",
		blob_id, node_count, pos->column, pos->row,
		blob_offset.x, blob_offset.y);

	svg_open_path(out_stream, blob_id, color, NULL, 0);

	for (node = 0, point_p.t = 0; node < node_count; node++) {
		struct point_c point_c;
		float sector_limit = (node + 1) * 360 / node_count;
		float sector_start;
		struct point_c final;

		sector_start = point_p.t + blob_params->sector_min;
		
		if (sector_start >= sector_limit) {
			error("node_%u: bad sector: {%f,%f}\n",
				node, sector_start, sector_limit);
			exit(EXIT_FAILURE);
		}

		point_p.t = random_float(sector_start, sector_limit);
		point_p.r = random_float(blob_params->radius_min,
			blob_params->radius_max);

		polar_to_cart(&point_p, &point_c);

		final.x = point_c.x + blob_offset.x;
		final.y = point_c.y + blob_offset.y;

		if (0) {
			fprintf(stderr,
				"  node_%u: {%f,%f} => {%f,%f} => {%f,%f}\n",
				node,
				point_p.r, point_p.t,
				point_c.x, point_c.y,
				final.x, final.y);
		}

		if (node == 0) {
			fprintf(out_stream, "   d=\"M %f,%f\n", final.x,
				final.y);
		} else {
			fprintf(out_stream, "    L %f,%f\n", final.x, final.y);
		}
	}

	fprintf(out_stream, "    Z\"\n");
	svg_close_object(out_stream);
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

static void write_svg(FILE* out_stream, const struct grid_params *grid_params,
	const struct blob_params *blob_params, const struct palette *palette,
	bool background)
{
	unsigned int i;
	unsigned int *render_order;
	struct grid_position pos;
	struct svg_rect background_rect;

	background_rect.width = (2 + grid_params->columns) * grid_params->width;
	background_rect.height = (2 + grid_params->rows) * grid_params->width;

	background_rect.x = -grid_params->width;
	background_rect.y = -grid_params->width;
	background_rect.rx = 50.0;

	svg_open_svg(out_stream, &background_rect);

	if (background) {
		//write_background(out_stream, &background_rect, "#001aff");
		write_background(out_stream, &background_rect, "#000099");
	}

	svg_open_group(out_stream, "camo_blobs");

	render_order = random_array(grid_params->columns * grid_params->rows);

	for (i = 0; i < grid_params->columns * grid_params->rows; i++) {
		pos.number = i;
		pos.row = render_order[i] / grid_params->columns;
		pos.column = render_order[i] % grid_params->columns;
		const char *color = palette_get_random(palette);

		//debug("%u: (%u) = %u, %u\n", i, render_order[i], pos.column, pos.row);
		write_blob(out_stream, grid_params, blob_params, color, &pos);
	}

	svg_close_group(out_stream);
	svg_close_svg(out_stream);
}

struct config_cb_data {
	const char *config_file;
	struct blob_params *blob_params;
	struct grid_params *grid_params;
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

		if (cbd->blob_params->node_count_min ==
			init_blob_params.node_count_min &&
			!strcmp(name, "blob_node_count_min")) {
			cbd->blob_params->node_count_min =
				to_unsigned(value);
		}
		if (cbd->blob_params->node_count_max ==
			init_blob_params.node_count_max &&
			!strcmp(name, "blob_node_count_max")) {
			cbd->blob_params->node_count_max =
				to_unsigned(value);
		}
		if (cbd->blob_params->radius_min ==
			init_blob_params.radius_min &&
			!strcmp(name, "blob_radius_min")) {
			cbd->blob_params->radius_min = to_float(value);
		}
		if (cbd->blob_params->radius_max ==
			init_blob_params.radius_max &&
			!strcmp(name, "blob_radius_max")) {
			cbd->blob_params->radius_max = to_float(value);
		}
		if (cbd->blob_params->sector_min ==
			init_blob_params.sector_min &&
			!strcmp(name, "blob_sector_min")) {
			cbd->blob_params->sector_min = to_float(value);
		}
		if (cbd->grid_params->columns == init_grid_params.columns &&
			!strcmp(name, "grid_columns")) {
			cbd->grid_params->columns = to_unsigned(value);
		}
		if (cbd->grid_params->rows == init_grid_params.rows &&
			!strcmp(name, "grid_rows")) {
			cbd->grid_params->rows = to_unsigned(value);
		}
		if (cbd->grid_params->width == init_grid_params.width &&
			!strcmp(name, "grid_width")) {
			cbd->grid_params->width = to_float(value);
		}
		if (cbd->grid_params->wiggle == init_grid_params.wiggle &&
			!strcmp(name, "grid_wiggle")) {
			cbd->grid_params->wiggle = to_float(value);
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

static const struct color_data default_colors[] = {
	{2, "#eeffff"},
	{2, "#bbbbbb"},
	{2, "#777777"},
	{1, "#97dcff"},
	{1, "#a3a3a3"},
	{2, "#00bbff"},
	{2, "#009aff"},
	{2, "#0077ff"},
	{1, "#004dff"},
	{1, "#003473"},
	{2, "#0000bb"},
	{2, "#000077"},
	{2, "#000011"},

	//{2, "#ff0000"},
	//{2, "#00ff00"},
	//{2, "#ffff00"},
	//{2, "#ff8800"},
	//{2, "#888800"},
	//{2, "#88ff00"},

};

static void get_config_opts(struct opts *opts, struct palette *palette)
{
	static const char *sections[] = {
		"[params]",
		"[palette]",
	};
	struct config_cb_data cbd = {
		.config_file = opts->config_file,
		.blob_params = &opts->blob_params,
		.grid_params = &opts->grid_params,
		.palette = palette,
	};

	config_process_file(opts->config_file, config_cb, &cbd,
		sections, sizeof(sections)/sizeof(sections[0]));
}

int main(int argc, char *argv[])
{
	struct opts opts;
	FILE *out_stream;
	struct palette palette = {0};

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
		get_config_opts(&opts, &palette);
	}

	if (!palette.color_count) {
		palette_fill(&palette, default_colors,
			sizeof(default_colors) / sizeof(default_colors[0]));
	}

	if (opts.blob_params.node_count_min ==
		init_blob_params.node_count_min) {
		opts.blob_params.node_count_min =
			default_blob_params.node_count_min;
	}
	if (opts.blob_params.node_count_max ==
		init_blob_params.node_count_max) {
		opts.blob_params.node_count_max =
			default_blob_params.node_count_max;
	}
	if (opts.blob_params.radius_min == init_blob_params.radius_min) {
		opts.blob_params.radius_min = default_blob_params.radius_min;
	}
	if (opts.blob_params.radius_max == init_blob_params.radius_max) {
		opts.blob_params.radius_max = default_blob_params.radius_max;
	}
	if (opts.blob_params.sector_min == init_blob_params.sector_min) {
		opts.blob_params.sector_min = default_blob_params.sector_min;
	}

	if (opts.grid_params.columns == init_grid_params.columns) {
		opts.grid_params.columns = default_grid_params.columns;
	}
	if (opts.grid_params.rows == init_grid_params.rows) {
		opts.grid_params.rows = default_grid_params.rows;
	}
	if (opts.grid_params.width == init_grid_params.width) {
		opts.grid_params.width = 1.1 * opts.blob_params.radius_max;
	}
	if (opts.grid_params.wiggle == init_grid_params.wiggle) {
		opts.grid_params.wiggle = 0.8 * opts.blob_params.radius_max;
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

	write_svg(out_stream, &opts.grid_params, &opts.blob_params, &palette,
		opts.background);

	mem_free(palette.colors);

	return EXIT_SUCCESS;
}

