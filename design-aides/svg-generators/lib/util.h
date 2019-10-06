/*
 *  moto-design SGV utils.
 */

#if ! defined(_MD_GENERATOR_UTIL_H)
#define _MD_GENERATOR_UTIL_H

#include <stdbool.h>

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
