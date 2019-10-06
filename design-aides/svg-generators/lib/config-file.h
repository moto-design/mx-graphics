/*
 *  moto-design SGV utils.
 */

#if ! defined(_MD_GENERATOR_CONFIG_FILE_H)
#define _MD_GENERATOR_CONFIG_FILE_H

typedef void (*config_file_callback)(void *cb_data, const char *section,
	char *config_data);

char *config_clean_data(char *p);
void config_process_file(const char *config_file, config_file_callback cb,
	void *cb_data, const char * const*sections, unsigned int section_count);

#define cbd_set_value(_cbd, _init, _name, _prefix, _param, _value) do { \
	if (_cbd->_param == _init._param && !strcmp(_name, #_prefix #_param)) { \
		debug("set from config: " #_param "\n"); \
		_cbd->_param = _value; \
	} \
} while(0)

#define opts_set_value(_param, _value, _error) do { \
	_param = _value; \
	if (_param == _error) { \
		opts->help = opt_yes; \
		return -1; \
	} \
} while(0)

#define opts_set_default(_opts, _init, _default, _param) do { \
	if (_opts._param == _init._param) { \
		debug("set from default: " #_param "\n"); \
		_opts._param = _default._param; \
	} \
} while(0)

#endif /* _MD_GENERATOR_CONFIG_FILE_H */
