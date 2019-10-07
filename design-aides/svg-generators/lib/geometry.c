/*
 *  moto-design SGV utils.
 */

#define _GNU_SOURCE
#define _ISOC99_SOURCE

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <fenv.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"
#include "log.h"

struct point_c *polar_to_cart(const struct point_p *p, struct point_c *c)
{
	int fe_err;
	float rad = deg_to_rad(p->t);

	errno = 0;
	feclearexcept(FE_ALL_EXCEPT);

	c->x = p->r * cosf(rad);
	fe_err = fetestexcept(FE_INVALID);

	c->y = p->r * sinf(rad);
	fe_err |= fetestexcept(FE_INVALID);

	if (errno || fe_err) {
		error("Math error: %d (%s)\n", fe_err, strerror(errno));
		assert(0);
		exit(EXIT_FAILURE);
	}

	return c;
}

struct point_p *cart_to_polar(const struct point_c *c, struct point_p *p)
{
	int fe_err;

	errno = 0;
	feclearexcept(FE_ALL_EXCEPT);

	p->r = sqrtf(c->x * c->x + c->y * c->y);
	fe_err = fetestexcept(FE_INVALID);

	p->t = deg_to_rad(atan2f(c->y, c->x));
	fe_err |= fetestexcept(FE_INVALID);

	if (errno || fe_err) {
		error("Math error: %d (%s)\n", fe_err, strerror(errno));
		assert(0);
		exit(EXIT_FAILURE);
	}

	return p;
}

void debug_print_cart(const char *msg, const struct point_c *c)
{
	debug("%scart:   x = %f, y = %f\n", msg, c->x, c->y);
}

void debug_print_polar(const char *msg, const struct point_p *p)
{
	debug("%spolar: r = %f, t = %f\n", msg, p->r, p->t);
}

void debug_print_pc(const char *msg, const struct point_pc *pc)
{
	debug_print_polar(msg, &pc->p);
	debug_print_cart(msg, &pc->c);
}

void debug_print_segment(const char *msg, const struct segment_c *seg)
{
	debug("%sa         = {%f, %f}\n", msg, seg->a.x, seg->a.y);
	debug("%sb         = {%f, %f}\n", msg, seg->b.x, seg->b.y);
	debug("%sslope     = %f\n", msg, seg->slope);
	debug("%sintercept = %f\n", msg, seg->intercept);
}

struct point_c segment_intersection(const struct segment_c *seg1,
	const struct segment_c *seg2)

{
	static const float s_diff_range = 0.1;
	const float s_diff = seg1->slope - seg2->slope;
	struct point_c i;

	debug("slope diff = %f\n", s_diff);

	if (s_diff > -s_diff_range && s_diff < s_diff_range) {
		error("No intersection (%f).\n", s_diff);
		debug_print_segment("seg1 ", seg1);
		debug_print_segment("seg2 ", seg2);
		assert(0);
		exit(EXIT_FAILURE);
	}

	i.x = (seg2->intercept - seg1->intercept) / (s_diff);
	i.y = seg1->slope * i.x + seg1->intercept;
	
	debug_print_cart("intersection ", &i);

	return i;
}
