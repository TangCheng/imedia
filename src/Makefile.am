## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(IMEDIA_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bindir = $(prefix)/imedia
bin_PROGRAMS = imedia

imedia_SOURCES = \
	imedia.c \
	main.c

imedia_LDFLAGS = 

imedia_LDADD = $(IMEDIA_LIBS)


SUBDIRS = \
	config
