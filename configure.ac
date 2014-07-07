dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(imedia, 0.1.00)

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AC_PROG_CC








LT_INIT


PKG_CHECK_MODULES(IMEDIA, [json-glib-1.0 glib-2.0 gobject-2.0 libipcam_base-0.1.0 shm_queue-0.1.00 ])

AC_ARG_ENABLE([hi3518],
	AS_HELP_STRING([--enable-hi3518],[Build with hi3518 support (default: enabled)]),
	[enable_hi3518=$enableval],
	[enable_hi3518=yes])

AC_ARG_ENABLE([hi3516],
	AS_HELP_STRING([--enable-hi3516],[Build with hi3516 support (default: disabled)]),
	[enable_hi3516=$enableval],
	[enable_hi3516=no])

if test "x$enable_hi3518" = "xyes" && test "x$enable_hi3516" = "xyes"; then
   AC_MSG_ERROR([Can not enable hi3518 and hi3516 at same time.])
fi

AM_CONDITIONAL([HI3518], [test x"$enable_hi3518" = "xyes"])
AM_CONDITIONAL([HI3516], [test x"$enable_hi3516" = "xyes"])

if test "x$enable_hi3518" = "xyes" || test "x$enable_hi3516" = "xyes"; then
trymppdir=""
AC_ARG_WITH(hisimpp,
       [  --with-hisimpp=PATH     Specify path to hisi mpp installation ],
       [
                if test "x$withval" != "xno" ; then
                        trymppdir=$withval
                fi
       ]
)
 
dnl ------------------------------------------------------
dnl hisimpp detection.  swiped from Tor.  modified a bit.
 
AC_CACHE_CHECK([for hisimpp directory], ac_cv_hisimpp_dir, [
  saved_LIBS="$LIBS"
  saved_LDFLAGS="$LDFLAGS"
  saved_CPPFLAGS="$CPPFLAGS"
  mpp_found=no
  for mppdir in $trymppdir "" $prefix /usr/local ; do
    LDFLAGS="$saved_LDFLAGS"
    LIBS="$saved_LIBS $mppdir/lib/libmpi.a $mppdir/lib/lib_hiae.a $mppdir/lib/libanr.a $mppdir/lib/libaec.a $mppdir/lib/lib_hiaf.a $mppdir/lib/lib_hiawb.a $mppdir/lib/libmem.a $mppdir/lib/libtde.a $mppdir/lib/libVoiceEngine.a $mppdir/lib/libresampler.a $mppdir/lib/libisp.a -lpthread"
 
    # Skip the directory if it isn't there.
    if test ! -z "$mppdir" -a ! -d "$mppdir" ; then
       continue;
    fi
    if test ! -z "$mppdir" ; then
      if test -d "$mppdir/lib" ; then
        LDFLAGS="-L$mppdir/lib $LDFLAGS"
      else
        LDFLAGS="-L$mppdir $LDFLAGS"
      fi
      if test -d "$mppdir/include" ; then
        CPPFLAGS="-I$mppdir/include $CPPFLAGS"
      else
        CPPFLAGS="-I$mppdir $CPPFLAGS"
      fi
    fi
    # Can I compile and link it?
    AC_TRY_LINK([#include <hi_comm_sys.h>
#include <mpi_sys.h>], [ HI_MPI_SYS_Exit(); ],
       [ hisimpp_linked=yes ], [ hisimpp_linked=no ])
    if test $hisimpp_linked = yes; then
       if test ! -z "$mppdir" ; then
         ac_cv_hisimpp_dir=$mppdir
       else
         ac_cv_hisimpp_dir="(system)"
       fi
       mpp_found=yes
       break
    fi
  done
  LIBS="$saved_LIBS"
  LDFLAGS="$saved_LDFLAGS"
  CPPFLAGS="$saved_CPPFLAGS"
  if test $mpp_found = no ; then
    AC_MSG_ERROR([hisimpp is required.
 
      If it's already installed, specify its path using --with-hisimpp=/dir/
])
  fi
])
dnl LIBS="$LIBS"
if test $ac_cv_hisimpp_dir != "(system)"; then
  if test -d "$ac_cv_hisimpp_dir/lib" ; then
    dnl LDFLAGS="-L$ac_cv_hisimpp_dir/lib $LDFLAGS"
    MPPLIBDIR="$ac_cv_hisimpp_dir/lib"
  else
    dnl LDFLAGS="-L$ac_cv_hisimpp_dir $LDFLAGS"
    MPPLIBDIR="$ac_cv_hisimpp_dir"
  fi
  AC_SUBST(MPPLIBDIR)
  if test -d "$ac_cv_hisimpp_dir/include" ; then
    CPPFLAGS="-I$ac_cv_hisimpp_dir/include $CPPFLAGS"
  else
    CPPFLAGS="-I$ac_cv_hisimpp_dir $CPPFLAGS"
  fi
fi

fi

AC_OUTPUT([
Makefile
src/Makefile
src/config/Makefile

])
