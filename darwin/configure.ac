#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(darwin, 0.5.1, darwin@eckerd.edu)
AM_INIT_AUTOMAKE(darwin, 0.5.1)
AC_CONFIG_SRCDIR([config.h])
AC_CONFIG_HEADERS([config.h])


# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_YACC
# autoscan says add following wanted by ltmain.sh
AC_PROG_LN_S
#AC_PROG_LIBTOOL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lInterface':
AC_CHECK_LIB([Interface], [AboutDialog::AboutDialog])
# FIXME: Replace `main' with a function in `-lMatching':
AC_CHECK_LIB([Matching], [Match::Match])
# FIXME: Replace `main' with a function in `-lWLC':
AC_CHECK_LIB([WLC], [WL_SetErrorMsg])
# JHS - following found by autoscan - mistake????
# FIXME: Replace `main' with a function in `-lc':
# AC_CHECK_LIB([c], [main])
# FIXME: Replace `main' with a function in `-limage_processing':
AC_CHECK_LIB([image_processing], [convColorToPixmapString])
# FIXME: Replace `main' with a function in `-lmath':
AC_CHECK_LIB([math], [gaussj])

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_CHECK_HEADERS([argz.h fcntl.h float.h inttypes.h langinfo.h libintl.h limits.h locale.h malloc.h stddef.h stdint.h stdio_ext.h stdlib.h string.h strings.h sys/param.h unistd.h wchar.h wctype.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])

#added from old configure.ac - JHS
AM_PATH_GTK_2_0(2.0.0, ,
            AC_MSG_ERROR(Cannot find GTK: Is pkg-config in path?))

# Checks for library functions.
AM_GNU_GETTEXT
# AM_GLIB_GNU_GETTEXT
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify floor getcwd mempcpy memset munmap nl_langinfo pow setlocale sqrt stpcpy strcasecmp strchr strcspn strdup strrchr strstr strtoul])

dnl -------------------------------------------
dnl Copy of OLD stuff from configure.in -- JHS

dnl Set PACKAGE_LOCALE_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, ${ac_default_prefix}/${DATADIRNAME}/locale)
else
  AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, ${prefix}/${DATADIRNAME}/locale)
fi

dnl Set PACKAGE_DATA_DIR in config.h.
if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED([PACKAGE_DATA_DIR], "${ac_default_prefix}/share/${PACKAGE}")
  else
    AC_DEFINE_UNQUOTED([PACKAGE_DATA_DIR], "${prefix}/share/${PACKAGE}")
  fi
else
  AC_DEFINE_UNQUOTED([PACKAGE_DATA_DIR], "${datadir}/${PACKAGE}")
fi

dnl Set PACKAGE_SOURCE_DIR in config.h.
packagesrcdir=`cd $srcdir && pwd`
AC_DEFINE_UNQUOTED([PACKAGE_SOURCE_DIR], "${packagesrcdir}")

dnl Set build and host info for config.h
AC_DEFINE([BUILD_ALIAS], [$build_alias])
AC_DEFINE_UNQUOTED([HOST_ALIAS], "$host_alias")

AC_DEFINE_UNQUOTED([HOST_CPU], "$host_cpu")
AC_DEFINE_UNQUOTED([BUILD_CPU], "$build_cpu")

AC_DEFINE_UNQUOTED([HOST_OS], "$host_os")
AC_DEFINE_UNQUOTED([BUILD_OS], "$build_os")

AC_DEFINE_UNQUOTED([HOST_VENDOR], "$host_vendor")
AC_DEFINE_UNQUOTED([BUILD_VENDOR], "$build_vendor")

AC_DEFINE_UNQUOTED([TARGET_ALIAS], "$target_alias")

dnl Use -Wall if we have gcc.
changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl

dnl -------------------------------------------
AC_CONFIG_FILES([po/Makefile.in 
                 m4/Makefile 
                 intl/Makefile
                 src/Makefile
                 src/image_processing/Makefile
                 src/interface/Makefile
                 src/matching/Makefile
                 src/math/Makefile
                 src/wavelet/Makefile
                 Makefile])
AC_OUTPUT
