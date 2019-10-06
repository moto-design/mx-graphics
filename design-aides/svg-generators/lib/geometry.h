/*
 *  moto-design SGV utils.
 */

#if ! defined(_MD_GENERATOR_GEOMETRY_H)
#define _MD_GENERATOR_GEOMETRY_H

#include <math.h>

static inline float deg_to_rad(float deg)
{
	return deg * M_PI / 180.0;
}

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

struct segment_c {
	struct point_c a;
	struct point_c b;
};

struct segment_p {
	struct point_p a;
	struct point_p b;
};

struct point_c *segment_intersection(const struct segment_c *s1,
	const struct segment_c *s2, struct point_c i);

#endif /* _MD_GENERATOR_GEOMETRY_H */
