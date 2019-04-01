/*
 *  moto-design random image generator.
 */

#if ! defined(_MD_GENERATOR_UTIL_H)
#define _MD_GENERATOR_UTIL_H

void __attribute__((unused)) __attribute__ ((format (printf, 3, 4)))
	_error(const char *func, int line, const char *fmt, ...);
void __attribute__((unused)) __attribute__ ((format (printf, 3, 4)))
	_log(const char *func, int line, const char *fmt, ...);
void __attribute__((unused)) __attribute__ ((format (printf, 3, 4)))
	_warn(const char *func, int line, const char *fmt, ...);

void set_verbose(bool state);
#if defined(DEBUG)
# define debug(_args...) do {_log(__func__, __LINE__, _args);} while(0)
#else
# define debug(...) do {} while(0)
#endif
# define error(_args...) do {_error(__func__, __LINE__, _args);} while(0)
# define log(_args...) do {_log(__func__, __LINE__, _args);} while(0)
# define warn(_args...) do {_warn(__func__, __LINE__, _args);} while(0)

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
	const char *fill, const char *stroke);
void svg_close_object(FILE *stream);
void svg_open_path(FILE *stream, const char *id, const char *fill,
	const char *stroke);
void svg_write_rect(FILE *stream, const char *id, const char *fill,
	const char *stroke, const struct svg_rect *rect);

struct point_c {
	float x;
	float y;
};

struct point_p {
	float radius;
	float angle;
};

float deg_to_rad(float deg);
void polar_to_cart(const struct point_p *p, struct point_c *c);

typedef void (*config_file_callback)(void *cb_data, const char *section,
	char *config_data);

char *config_clean_data(char *p);
void config_process_file(const char *config_file, config_file_callback cb,
	void *cb_data, const char * const*sections, unsigned int section_count);

#endif /* _MD_GENERATOR_UTIL_H */
