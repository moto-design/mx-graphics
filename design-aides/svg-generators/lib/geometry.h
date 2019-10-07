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

void debug_print_cart(const char *msg, const struct point_c *c);
void debug_print_polar(const char *msg, const struct point_p *p);
void debug_print_pc(const char *msg, const struct point_pc *pc);

struct segment_c {
	struct point_c a;
	struct point_c b;
	float slope;
	float intercept;
};

struct segment_p {
	struct point_p a;
	struct point_p b;
};

void debug_print_segment(const char *msg, const struct segment_c *seg);

static inline float segment_slope(struct segment_c *seg)
{	
	return (seg->b.y - seg->a.y) / (seg->b.x - seg->a.x);
}

static inline float segment_intercept(struct segment_c *seg)
{
	return seg->a.y - (seg->slope * seg->a.x);
}

static inline struct segment_c *segment_init(struct segment_c *seg,
	const struct point_c *a, const struct point_c *b)
{
	seg->a = *a;
	seg->b = *b;
	seg->slope = segment_slope(seg);
	seg->intercept = segment_intercept(seg);
	
	return seg;
}

struct point_c segment_intersection(const struct segment_c *seg1,
	const struct segment_c *seg2);

#endif /* _MD_GENERATOR_GEOMETRY_H */
