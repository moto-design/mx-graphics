/*
 *  moto-design SGV utils.
 */

#define _GNU_SOURCE
#define _ISOC99_SOURCE

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "svg.h"

void svg_open_svg(FILE *stream, const struct svg_rect *background_rect)
{
	fprintf(stream, "<svg \n"
		"  xmlns=\"http://www.w3.org/2000/svg\"\n"
		"  xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\n"
//		"  width=\"%f\"\n"
//		"  height=\"%f\"\n"
		"  viewBox=\"%f %f %f %f\">\n",
//		background_rect->width, background_rect->width,
		background_rect->x, background_rect->y,
		background_rect->width, background_rect->height);
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
	const char *fill, const char *stroke, unsigned int stroke_width)
{
	fprintf(stream, "  <%s id=\"%s\"\n", type, id);
	if (fill) {
		fprintf(stream, "   fill=\"%s\"\n", fill);
	}
	if (stroke) {
		fprintf(stream, "   stroke=\"%s\" stroke-width=\"%u\"\n", stroke, stroke_width);
	}
}

void svg_close_object(FILE *stream)
{
	fprintf(stream, "  />\n");
}

void svg_open_path(FILE *stream, const char *id, const char *fill,
	const char *stroke, unsigned int stroke_width)
{
	svg_open_object(stream, "path", id, fill, stroke, stroke_width);
}

void svg_open_polygon(FILE *stream, const char *id, const char *fill,
	const char *stroke, unsigned int stroke_width)
{
	svg_open_object(stream, "polygon", id, fill, stroke, stroke_width);
	fprintf(stream, "   points=\"\n");
}

void svg_close_polygon(FILE *stream)
{
	fprintf(stream, "   \"\n");
}

void svg_write_rect(FILE *stream, const char *id, const char *fill,
	const char *stroke, unsigned int stroke_width,
	const struct svg_rect *rect)
{
	svg_open_object(stream, "rect", id, fill, stroke, stroke_width);

	fprintf(stream,
		"   width=\"%f\"\n   height=\"%f\"\n   x=\"%f\"\n   y=\"%f\"\n   rx=\"%f\"\n",
		rect->width, rect->height, rect->x, rect->y, rect->rx);

	svg_close_object(stream);
}

