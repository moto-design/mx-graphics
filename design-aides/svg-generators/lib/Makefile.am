svg_include = $(srcdir)

AM_CPPFLAGS = -I$(svg_include) $(DEFAULT_CPPFLAGS)
AM_CFLAGS = $(DEFAULT_CFLAGS)

noinst_LTLIBRARIES = libsvg-utils.la

libsvg_utils_la_SOURCES = \
	svg-utils.h \
	config-file.c config-file.h \
	geometry.c geometry.h \
	log.c log.h \
	mem.c mem.h \
	svg.c svg.h \
	util.c util.h

libsvg_utils_la_LIBADD = -lm

MAINTAINERCLEANFILES = Makefile.in
