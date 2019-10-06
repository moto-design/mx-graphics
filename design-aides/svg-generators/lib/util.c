/*
 *  moto-design SGV utils.
 */

#define _GNU_SOURCE
#define _ISOC99_SOURCE

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "log.h"
#include "mem.h"
#include "util.h"

const char *eat_front_ws(const char *p)
{
	//char *const start = p;

	assert(p);

	while (*p && (*p == ' ' || *p == '\t' || *p == '\r')) {
		//debug("'%s': remove='%c'\n",
		//	start, *p);
		p++;
	}

	return p;
}

void eat_tail_ws(char *p)
{
	char *const start = p;

	assert(p);

	//debug("> '%s'\n", p);
	while (*p) {
		p++;
	}
	p--;

	while (p > start && (*p == ' ' || *p == '\t' || *p == '\r')) {
		//debug("'%s': remove='%c'\n",
		//	start, *p);
		p--;
	}
	
	*(p + 1) = 0;
	//debug("< '%s'\n", start);
}

unsigned int to_unsigned(const char *str)
{
	const char *p;
	unsigned long u;

	for (p = str; *p; p++) {
		if (!isdigit(*p)) {
			error("isdigit failed: '%s'\n", str);
			return UINT_MAX;
		}
	}

	u = strtoul(str, NULL, 10);

	if (u == ULONG_MAX) {
		error("strtoul '%s' failed: %s\n", str, strerror(errno));
		return UINT_MAX;
	}
	
	if (u > UINT_MAX) {
		error("too big: %lu\n", u);
		return UINT_MAX;
	}

	return (unsigned int)u;
}

float to_float(const char *str)
{
	const char *p;
	const char *start;
	char *end;
	float f;
	bool found_decimal = false;

	assert(str);
	set_verbose(true);
	debug("str = @%s@\n", str);

	start = eat_front_ws(str);

	for (p = (*start != '-') ? start : start + 1; *p; p++) {
		if (*p == '-') {
			error("bad sign: '%s'\n", str);
			return HUGE_VALF;
		}
		if (*p == '.') {
			if (found_decimal) {
				error("multiple decimal: '%s'\n", str);
				return HUGE_VALF;
			}
			found_decimal = true;
			continue;
		}		
		if (!isdigit(*p)) {
			error("isdigit failed: '%s'\n", str);
			return HUGE_VALF;
		}
	}

	f = strtof(start, &end);

	if ((f == 0.0 && end == str) || f == HUGE_VALF) {
		error("strtof '%s' failed: %s\n", str, strerror(errno));
		return HUGE_VALF;
	}

	return f;
}

int random_int(int min, int max)
{
	return min + (rand() % (max - min + 1));
}

unsigned int random_unsigned(unsigned int min, unsigned int max)
{
	return (unsigned int)(min + (rand() % (max - min + 1)));
}

float random_float(float min, float max)
{
    return min + (float)rand() / (float)RAND_MAX * (max - min);
}

void hex_color_set_value(char *color, const char *value)
{
	debug("value = '%s'\n", value);
	assert(color);

	if (value && !is_hex_color(value) ) {
		fprintf(stderr, "Bad hex color value: '%s'\n", value);
		assert(0);
		exit(EXIT_FAILURE);
	}

	memcpy(color, value, hex_color_len);
}

void palette_fill(struct palette *palette, const struct color_data *data,
	unsigned int data_len)
{
	unsigned int i;
	unsigned int out;

	if (palette->colors) {
		mem_free(palette->colors);
		palette->colors = NULL;
	}
	
	for (i = 0, palette->color_count = 0; i < data_len; i++) {
		palette->color_count += data[i].weight;
	}

	palette->colors =  mem_alloc(palette->color_count * hex_color_len);

	for (i = 0, out = 0; i < data_len; i++) {
		unsigned int j;
		for (j = 0; j < data[i].weight; j++, out++) {
			debug("Add %s\n", data[i].value);
			memcpy(&palette->colors[out], data[i].value,
				hex_color_len);
		}
		
	}
}

const char *palette_get_random(const struct palette *palette)
{
	return palette->colors[random_unsigned(0, palette->color_count - 1)];
}

unsigned int *random_array(unsigned int len)
{
	unsigned int *p;
	unsigned int i;

	p = mem_alloc(len * sizeof(*p));

	for (i = 0; i < len; i++) {
		p[i] = i;
	}

	for (i = 0; i < len; i++) {
		unsigned int j;
		unsigned int tmp;

		j = rand() % len;
		tmp = p[i];
		p[i] = p[j];
		p[j] = tmp;
	}

	return p;
}

bool is_hex_color(const char *str)
{
	assert(str);

	return (*str == '#'
		&& *(str + 1) && isxdigit(*(str + 1))
		&& *(str + 2) && isxdigit(*(str + 2))
		&& *(str + 3) && isxdigit(*(str + 3))
		&& *(str + 4) && isxdigit(*(str + 4))
		&& *(str + 5) && isxdigit(*(str + 5))
		&& *(str + 6) && isxdigit(*(str + 6))
		&& *(str + 7) == 0);
}

struct stroke *stroke_set(struct stroke *stroke, const char *color,
	unsigned int width)
{
	debug("color = '%s', width = %u\n", color, width);

	stroke->width = width;
	hex_color_set_value(stroke->color, color);
	return stroke;
}

struct fill *fill_set(struct fill *fill, const char *color)
{
	debug("color = '%s'\n", color);

	hex_color_set_value(fill->color, color);
	return fill;
}
