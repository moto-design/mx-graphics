/*
 *  moto-design SGV utils.
 */

#if ! defined(_MD_GENERATOR_UTIL_H)
#define _MD_GENERATOR_UTIL_H

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "log.h"

void *mem_alloc(size_t size);
void *mem_realloc(void *p, size_t size);
void mem_free(void *p);

const char *eat_front_ws(const char *p);
void eat_tail_ws(char *p);

unsigned int to_unsigned(const char *str);
float to_float(const char *str);

int random_int(int min, int max);
unsigned int random_unsigned(unsigned int min, unsigned int max);
float random_float(float min, float max);
unsigned int *random_array(unsigned int len);

bool is_hex_color(const char *p);
#define hex_color_len sizeof("#000000")
void hex_color_set_value(char *color, const char *value);

struct color_data
{
	unsigned int weight;
	char value[hex_color_len];
};

struct palette
{
	unsigned int color_count;
	char (*colors)[hex_color_len];
};

void palette_parse_config(const char *config_file, struct palette *palette);
void palette_fill(struct palette *palette, const struct color_data *data,
	unsigned int data_len);
const char *palette_get_random(const struct palette *palette);


struct svg_rect {
	float width;
	float height;
	float x;
	float y;
	float rx;
};

void svg_open_svg(FILE *stream, const struct svg_rect *background_rect);
void svg_close_svg(FILE *stream);
void svg_open_group(FILE *stream, const char *id);
void svg_close_group(FILE *stream);
void svg_open_object(FILE *stream, const char *type, const char *id,
	const char *fill, const char *stroke, unsigned int stroke_width);
void svg_close_object(FILE *stream);
void svg_open_path(FILE *stream, const char *id, const char *fill,
	const char *stroke, unsigned int stroke_width);
void svg_open_polygon(FILE *stream, const char *id, const char *fill,
	const char *stroke, unsigned int stroke_width);
void svg_close_polygon(FILE *stream);
void svg_write_rect(FILE *stream, const char *id, const char *fill,
	const char *stroke, unsigned int stroke_width, const struct svg_rect *rect);

struct point_c {
	float x;
	float y;
};

struct point_p {
	float r;
	float t;
};

struct point_pc {
	struct point_p p;
	struct point_c c;
};

float deg_to_rad(float deg);

struct point_c *polar_to_cart(const struct point_p *p, struct point_c *c);
struct point_p *cart_to_polar(const struct point_c *c, struct point_p *p);

static inline struct point_c *pc_polar_to_cart(struct point_pc *pc) {
	return polar_to_cart(&pc->p, &pc->c);
};
static inline struct point_p *pc_cart_to_polar(struct point_pc *pc) {
	return cart_to_polar(&pc->c, &pc->p);
};

void debug_print_cart(const struct point_c *c);
void debug_print_polar(const struct point_p *p);
void debug_print_pc(const struct point_pc *pc);

typedef void (*config_file_callback)(void *cb_data, const char *section,
	char *config_data);

char *config_clean_data(char *p);
void config_process_file(const char *config_file, config_file_callback cb,
	void *cb_data, const char * const*sections, unsigned int section_count);

#define cbd_set_value(_cbd, _init, _name, _prefix, _param, _value) do { \
	if (_cbd->_param == _init._param && !strcmp(_name, #_prefix #_param)) { \
		debug("set from config: " #_param "\n"); \
		_cbd->_param = _value; \
	} \
} while(0)

#define opts_set_value(_param, _value, _error) do { \
	_param = _value; \
	if (_param == _error) { \
		opts->help = opt_yes; \
		return -1; \
	} \
} while(0)

#define opts_set_default(_opts, _init, _default, _param) do { \
	if (_opts._param == _init._param) { \
		debug("set from default: " #_param "\n"); \
		_opts._param = _default._param; \
	} \
} while(0)

static inline float min_f(float a, float b)
{
	return a < b ? a : b;
}

static inline float max_f(float a, float b)
{
	return a > b ? a : b;
}

struct stroke {
	unsigned int width;
	char color[hex_color_len];
};

struct stroke *stroke_set(struct stroke *stroke, const char *color,
	unsigned int width);

struct fill {
	char color[hex_color_len];
};

struct fill *fill_set(struct fill *fill, const char *color);

#endif /* _MD_GENERATOR_UTIL_H */
