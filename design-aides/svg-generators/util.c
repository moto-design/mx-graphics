/*
 *  moto-design random image generator.
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
#include <stdarg.h>
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

bool verbose = false;

void set_verbose(bool state)
{
	verbose = state;
}

void  __attribute__((unused)) _error(const char *func, int line,
	const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "ERROR: %s:%d: ", func, line);

	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);
	fflush(stderr);

	va_end(ap);
}

void  __attribute__((unused)) _log(const char *func, int line,
	const char *fmt, ...)
{
	va_list ap;
	
	if (!verbose) {
		return;
	}

	fprintf(stderr, "%s:%d: ", func, line);

	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);
	fflush(stderr);

	va_end(ap);
}

void  __attribute__((unused)) _warn(const char *func, int line,
	const char *fmt, ...)
{
	va_list ap;

	fprintf(stderr, "WARNING: %s:%d: ", func, line);

	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);
	fflush(stderr);

	va_end(ap);
}

void *mem_alloc(size_t size)
{
	void *p = malloc(size);

	if (!p) {
		error("malloc %lu failed: %s.\n", (unsigned long)size,
			strerror(errno));
		assert(0);
		exit(EXIT_FAILURE);
	}
	memset(p, 0, size);
	return p;
}

void *mem_realloc(void *p, size_t size)
{
	void *n = realloc(p, size);

	if (!n) {
		error("realloc %lu failed: %s.\n", (unsigned long)size,
			strerror(errno));
		assert(0);
		exit(EXIT_FAILURE);
	}
	return n;
}

void mem_free(void *p)
{
	if (!p) {
		error("null free.\n");
		assert(0);
		exit(EXIT_FAILURE);
	}

	free(p);
}

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

	for (p = start = eat_front_ws(str); *p; p++) {
		if (*p == '-' && p != start) {
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

	f = strtof(str, &end);

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

void svg_open_svg(FILE *stream, const struct svg_rect *background_rect)
{
	fprintf(stream, "<svg \n"
		"  xmlns=\"http://www.w3.org/2000/svg\"\n"
		"  xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\n"
		"  width=\"%f\"\n"
		"  height=\"%f\"\n"
		"  viewBox=\"%f %f %f %f\">\n",
		background_rect->width, background_rect->width,
		background_rect->x, background_rect->y,
		background_rect->width, background_rect->width);
}

void svg_close_svg(FILE *stream)
{
	fprintf(stream, "</svg>\n");
}

void svg_open_group(FILE *stream, const char *id)
{
	fprintf(stream,
		" <g  id=\"%s\" inkscape:label=\"%s\" inkscape:groupmode=\"layer\">\n",
		id, id);
}

void svg_close_group(FILE *stream)
{
	fprintf(stream, " </g>\n");
}

void svg_open_object(FILE *stream, const char *type, const char *id,
	const char *fill, const char *stroke)
{
	//static const char debug_stroke[]=";stroke:#000000;stroke-width:0.5";
	static const char debug_stroke[]="";

	(void)stroke;

	fprintf(stream, "  <%s id=\"%s\" style=\"fill:%s%s\"\n", type, id, fill,
		debug_stroke);
}

void svg_close_object(FILE *stream)
{
	fprintf(stream, "  />\n");
}

void svg_open_path(FILE *stream, const char *id, const char *fill,
	const char *stroke)
{
	svg_open_object(stream, "path", id, fill, stroke);
}

void svg_write_rect(FILE *stream, const char *id, const char *fill,
	const char *stroke, const struct svg_rect *rect)
{
	svg_open_object(stream, "rect", id, fill, stroke);

	fprintf(stream,
		"   width=\"%f\"\n   height=\"%f\"\n   x=\"%f\"\n   y=\"%f\"\n   rx=\"%f\"\n",
		rect->width, rect->height, rect->x, rect->y, rect->rx);

	svg_close_object(stream);
}

float deg_to_rad(float deg)
{
	return deg * M_PI / 180.0;
}

void polar_to_cart(const struct point_p *p, struct point_c *c)
{
	float rad = deg_to_rad(p->angle);
	
	c->x = p->radius * cosf(rad);
	c->y = p->radius * sinf(rad);
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
		&& *(str + 6) && isxdigit(*(str + 6)));
}

char *config_clean_data(char *p)
{
	char *start;

	assert(p);

	p = start = (char *)eat_front_ws(p);

	while (*p) {
		if (is_hex_color(p)) {
			p += 7;
			continue;
		}
		if (*p == '\n'  || *p == '\r' || *p == '#') {
			*p = 0;
			eat_tail_ws(start);
			return start;
		}
		p++;
	}
	eat_tail_ws(start);
	return start;
}

void config_process_file(const char *config_file, config_file_callback cb,
	void *cb_data, const char * const*sections, unsigned int section_count)
{
	FILE *fp;
	char buf[512];
	const char *current_section = NULL;

	fp = fopen(config_file, "r");

	if (!fp) {
		error("open config '%s' failed: %s\n", config_file,
		      strerror(errno));
		assert(0);
		exit(EXIT_FAILURE);
	}

	while (fgets(buf, sizeof(buf), fp)) {
		unsigned int i;
		char *p;

		p = config_clean_data(buf);

		if (!*p) {
			debug("null line\n");
			continue;
		}

		for (i = 0; i < section_count; i++) {
			const char *s = sections[i];

			if (!strcmp(p, s)) {
				debug("new section: %s => %s:\n", current_section, s);
				current_section = s;
				goto next_line;
			}
		}

		if (!current_section) {
			error("Bad config data '%s' (%s)\n", p, config_file);
			assert(0);
			exit(EXIT_FAILURE);
		}

		debug("cb: %s, '%s'\n", current_section, buf);
		cb(cb_data, current_section, buf);
next_line:
		(void)0;
	}

	debug("ON_EXIT\n");
	cb(cb_data, "ON_EXIT", NULL);
}


