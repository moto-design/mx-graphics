/*
 *  moto-design SGV utils.
 */

#if ! defined(_MD_GENERATOR_LOG_H)
#define _MD_GENERATOR_LOG_H

#include <stdbool.h>

void __attribute__((unused)) __attribute__ ((format (printf, 3, 4)))
	_error(const char *func, int line, const char *fmt, ...);
void __attribute__((unused)) __attribute__ ((format (printf, 3, 4)))
	_log(const char *func, int line, const char *fmt, ...);
void __attribute__((unused)) __attribute__ ((format (printf, 3, 4)))
	_warn(const char *func, int line, const char *fmt, ...);

void set_exit_on_error(bool state);
void set_verbose(bool state);

#if defined(DEBUG)
# define debug(_args...) do {_log(__func__, __LINE__, _args);} while(0)
#else
# define debug(...) do {} while(0)
#endif
# define error(_args...) do {_error(__func__, __LINE__, _args);} while(0)
# define log(_args...) do {_log(__func__, __LINE__, _args);} while(0)
# define warn(_args...) do {_warn(__func__, __LINE__, _args);} while(0)

#endif /* _MD_GENERATOR_LOG_H */
