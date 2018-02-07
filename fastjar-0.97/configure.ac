dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.61)
AC_INIT([fastjar],[0.97]) 
AC_CONFIG_SRCDIR([jartool.h])
AM_INIT_AUTOMAKE([-Wall gnu 1.10])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
gl_EARLY
AC_PROG_INSTALL
AC_EXEEXT

AC_DISABLE_STATIC

LT_INIT([win32-dll])
AC_SUBST(LIBTOOL_DEPS)

AM_MAINTAINER_MODE

AX_CFLAGS_WARN_ALL
AX_CFLAGS_GCC_OPTION([-pedantic])
AX_CFLAGS_GCC_OPTION([-W])
AX_CFLAGS_GCC_OPTION([-Wextra])
AX_CFLAGS_GCC_OPTION([-Wmissing-prototypes])
AX_CFLAGS_GCC_OPTION([-Wstrict-prototypes])
AX_CFLAGS_GCC_OPTION([-Wwrite-strings])

AC_PATH_PROG([FLAWFINDER],[flawfinder])
AC_PATH_PROG([PSCAN],[pscan])
AC_PATH_PROG([RATS],[rats])
AC_PATH_PROG([SPLINT],[splint])

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS_ONCE([fcntl.h unistd.h sys/param.h stdlib.h limits.h])

AC_CHECK_FUNCS_ONCE([ftruncate _chsize])

if test "$ac_cv_func_ftruncate" = no; then
  if test "$ac_cv_func__chsize" = no; then
    AC_MSG_ERROR([Need ftruncate or _chsize])
  fi
fi

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_STRUCT_TM

# mkdir takes a single argument on some systems.
gcc_AC_FUNC_MKDIR_TAKES_ONE_ARG

# check for fixed size types.
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

dnl Check byte order
AC_C_BIGENDIAN

ZLIBS=
use_zlib=maybe
AC_CHECK_LIB(z, deflate, ZLIBS=-lz, use_zlib=no)

if test "$use_zlib" = no; then
   # Brain dead way to find tree's zlib.
   AC_MSG_ERROR([Need zlib])
fi
AC_SUBST(ZLIBS)

gl_INIT

AC_CONFIG_FILES([lib/Makefile Makefile])
AC_OUTPUT
