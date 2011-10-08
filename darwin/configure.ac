#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.65)
AC_INIT([darwin],[2.22],[darwin@eckerd.edu])
AM_INIT_AUTOMAKE(darwin, 2.22)
#AC_CONFIG_SRCDIR([config.h])
AC_CONFIG_HEADERS([config.h])


# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_CXX
#AC_PROG_INSTALL
AC_PROG_RANLIB
#AC_PROG_YACC
# autoscan says add following wanted by ltmain.sh
#AC_PROG_LN_S
#AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([Interface], [AboutDialog::AboutDialog])
AC_CHECK_LIB([Matching], [Match::Match])
AC_CHECK_LIB([WLC], [WL_SetErrorMsg])
AC_CHECK_LIB([PNGsupport], [PngLoadImage])
AC_CHECK_LIB([image_processing], [convColorToPixmapString])
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
dnl AM_GNU_GETTEXT
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
dnl if test "x${prefix}" = "xNONE"; then
dnl   AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, ${ac_default_prefix}/${DATADIRNAME}/locale)
dnl else
dnl   AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, ${prefix}/${DATADIRNAME}/locale)
dnl fi

dnl NOTE: must run "./configure datadir=/usr/local/share" to set datadir
dnl Set PACKAGE_DATA_DIR in config.h.
if test "x${datadir}" = 'x${prefix}/share'; then
  if test "x${prefix}" = "xNONE"; then
    AC_DEFINE_UNQUOTED([PACKAGE_DATA_DIR], "${ac_default_prefix}/share/${PACKAGE}", 
                       [Define location of package data])
  else
    AC_DEFINE_UNQUOTED([PACKAGE_DATA_DIR], "${prefix}/share/${PACKAGE}", 
                       [Define location of package data])
  fi
else
   AC_DEFINE_UNQUOTED([PACKAGE_DATA_DIR], "${datadir}/${PACKAGE}", 
                      [Define location of package data])
fi

dnl Set PACKAGE_SOURCE_DIR in config.h.
packagesrcdir=`cd $srcdir && pwd`
dnl AC_DEFINE_UNQUOTED([PACKAGE_SOURCE_DIR], "${packagesrcdir}")
AC_DEFINE_UNQUOTED([PACKAGE_SOURCE_DIR], "${packagesrcdir}", [Define location of package source])

dnl Defines that need to go in config.h
AC_DEFINE([CURRENT_DBVERSION], [3], [Version number for Database Format])

dnl these are temorary for debugging autoreconf & configure (comment out later) - JHS
AC_DEFINE_UNQUOTED([DATADIR], ${datadir}, [Data Dir])
AC_DEFINE_UNQUOTED([PREFIX], ${prefix}, [Prefix])
AC_DEFINE_UNQUOTED([AC_DEFAULT_PREFIX], ${ac_default_prefix}, [AC Default Prefix])

dnl Set build and host info for config.h
dnl AC_DEFINE([BUILD_ALIAS], [$build_alias])
dnl AC_DEFINE_UNQUOTED([HOST_ALIAS], "$host_alias")

dnl AC_DEFINE_UNQUOTED([HOST_CPU], "$host_cpu")
dnl AC_DEFINE_UNQUOTED([BUILD_CPU], "$build_cpu")

dnl AC_DEFINE_UNQUOTED([HOST_OS], "$host_os")
dnl AC_DEFINE_UNQUOTED([BUILD_OS], "$build_os")

dnl AC_DEFINE_UNQUOTED([HOST_VENDOR], "$host_vendor")
dnl AC_DEFINE_UNQUOTED([BUILD_VENDOR], "$build_vendor")

dnl AC_DEFINE_UNQUOTED([TARGET_ALIAS], "$target_alias")

dnl Use -Wall if we have gcc.
dnl changequote(,)dnl
dnl if test "x$GCC" = "xyes"; then
dnl  case " $CFLAGS " in
dnl  *[\ \	]-Wall[\ \	]*) ;;
dnl  *) CFLAGS="$CFLAGS -Wall" ;;
dnl  esac
dnl fi
dnl changequote([,])dnl

dnl -------------------------------------------
dnl AC_CONFIG_FILES([
dnl              po/Makefile.in 
dnl              m4/Makefile 
dnl              intl/Makefile
AC_CONFIG_FILES([src/image_processing/Makefile
                 src/interface/Makefile
                 src/matching/Makefile
                 src/math/Makefile
                 src/wavelet/Makefile
                 src/Makefile
                 png/Makefile
                 Makefile])
AC_OUTPUT
