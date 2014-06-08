## Process this file with automake to produce Makefile.in

## Created by Anjuta

AUTOMAKE_OPTIONS = subdir-objects

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
	hi3518a/media_sys_ctrl.c \
	hi3518a/media_video_proc.c \
	imedia.c \
	main.c

imedia_LDFLAGS = 

imedia_LDADD = $(IMEDIA_LIBS) $(MPPLIBDIR)/libsns_nt99141.a $(MPPLIBDIR)/libmpi.a $(MPPLIBDIR)/lib_hiae.a $(MPPLIBDIR)/libanr.a $(MPPLIBDIR)/libaec.a $(MPPLIBDIR)/lib_hiaf.a $(MPPLIBDIR)/lib_hiawb.a $(MPPLIBDIR)/libmem.a $(MPPLIBDIR)/libtde.a $(MPPLIBDIR)/libVoiceEngine.a $(MPPLIBDIR)/libresampler.a $(MPPLIBDIR)/libisp.a

SUBDIRS = \
	config
