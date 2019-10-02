AUTOMAKE_OPTIONS = foreign

SUBDIRS = svg-utils
svg_lib = svg-utils/libsvg-utils.la
svg_include = $(srcdir)/svg-utils

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I$(svg_include) $(DEFAULT_CPPFLAGS)
AM_CFLAGS = $(DEFAULT_CFLAGS)

LIBTOOL_DEPS = @LIBTOOL_DEPS@

EXTRA_DIST = bootstrap version.sh configure.ac $(srcdir)/m4 README.md

MAINTAINERCLEANFILES = autom4te.cache aclocal.m4 compile config.* configure \
 depcomp install-sh ltmain.sh Makefile.in missing $(PACKAGE)-*.gz

maintainer-clean-local:
	rm -rf m4

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

bin_PROGRAMS = blob-generator star-generator stripe-generator

blob_generator_DEPENDENCIES = Makefile
blob_generator_SOURCES = blob-generator.c
blob_generator_LDADD = $(svg_lib)

star_generator_DEPENDENCIES = Makefile
star_generator_SOURCES = star-generator.c
star_generator_LDADD = $(svg_lib)

stripe_generator_DEPENDENCIES = Makefile
stripe_generator_SOURCES = stripe-generator.c
stripe_generator_LDADD = $(svg_lib)

.PHONY: help

help:
	@echo "Targets:"
	@echo "  make install"
	@echo "  make dist"
	@echo "  make distcheck"
	@echo "  make distclean"
	@echo "  make maintainer-clean"
