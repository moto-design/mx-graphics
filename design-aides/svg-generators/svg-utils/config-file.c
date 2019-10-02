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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config-file.h"
#include "log.h"
#include "util.h"

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
