AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = $(DEFAULT_CPPFLAGS)
AM_CFLAGS = $(DEFAULT_CFLAGS)

EXTRA_DIST = version.sh configure.ac $(srcdir)/m4

MAINTAINERCLEANFILES = autom4te.cache aclocal.m4 compile config.* configure \
 depcomp install-sh ltmain.sh Makefile.in missing $(PACKAGE)-*.gz

maintainer-clean-local:
	rm -rf m4

bin_PROGRAMS = blob-generator star-generator stripe-generator

blob_generator_DEPENDENCIES = Makefile
blob_generator_SOURCES = util.c util.h blob-generator.c
blob_generator_LDADD = -lm

star_generator_DEPENDENCIES = Makefile
star_generator_SOURCES = util.c util.h star-generator.c
star_generator_LDADD = -lm

stripe_generator_DEPENDENCIES = Makefile
stripe_generator_SOURCES = util.c util.h stripe-generator.c
stripe_generator_LDADD = -lm

.PHONY: help

help:
	@echo "Targets:"
	@echo "  make install"
	@echo "  make dist"
	@echo "  make distcheck"
	@echo "  make distclean"
	@echo "  make maintainer-clean"
