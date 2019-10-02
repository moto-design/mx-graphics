/*
 *  moto-design SGV utils.
 */

#if ! defined(_MD_GENERATOR_SVG_H)
#define _MD_GENERATOR_SVG_H

#include <stdio.h>

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

#endif /* _MD_GENERATOR_SVG_H */
