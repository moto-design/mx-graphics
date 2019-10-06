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

void debug_print_cart(const struct point_c *c)
{
	debug("cart:    x = %f, y = %f\n", c->x, c->y);
}

void debug_print_polar(const struct point_p *p)
{
	debug("plolar: r = %f, t = %f\n", p->r, p->t);
}

void debug_print_pc(const struct point_pc *pc)
{
	debug_print_polar(&pc->p);
	debug_print_cart(&pc->c);
}

