## Process this file with automake to produce Makefile.in

## Created by Anjuta

AUTOMAKE_OPTIONS = subdir-objects

if INTERNAL_MPP
if HI3518
MPP_CFLAGS = -I$(top_srcdir)/src/hi3518/mpp2/include
MPP_LIBDIR = $(top_srcdir)/src/hi3518/mpp2/lib
endif
else
MPP_CFLAGS = $(MPPCFLAGS)
MPP_LIBDIR = $(MPPLIBDIR)
endif

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(IMEDIA_CFLAGS) \
	$(MPP_CFLAGS)

AM_CFLAGS =\
	-rdynamic\
	-Wall\
	-g

bindir = $(prefix)/imedia
bin_PROGRAMS = imedia

imedia_SOURCES = \
	osd_font/osd_font.c \
	media_sys_ctrl_interface.c \
	media_video_interface.c \
	media_osd_interface.c \
	imedia.c \
	main.c

if HI3518
imedia_SOURCES += \
	hi3518/media_sys_ctrl.c \
	hi3518/media_video.c \
	hi3518/media_osd.c \
	hi3518/isp.c \
	hi3518/video_encode.c \
	hi3518/video_input.c \
	hi3518/video_process_subsystem.c
endif

imedia_LDFLAGS = 

imedia_LDADD = $(IMEDIA_LIBS) $(MPP_LIBDIR)/libmpi.a $(MPP_LIBDIR)/lib_hiae.a $(MPP_LIBDIR)/libanr.a $(MPP_LIBDIR)/libaec.a $(MPP_LIBDIR)/lib_hiaf.a $(MPP_LIBDIR)/lib_hiawb.a $(MPP_LIBDIR)/libmem.a $(MPP_LIBDIR)/libtde.a $(MPP_LIBDIR)/libVoiceEngine.a $(MPP_LIBDIR)/libresampler.a $(MPP_LIBDIR)/libisp.a -ldl

SUBDIRS = \
	config
