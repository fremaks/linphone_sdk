#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.62])
AC_INIT([linphone_interface], [1.0], [fremaks@163.com])
AM_INIT_AUTOMAKE([linphone_interface], [1.0])
AC_CONFIG_SRCDIR([base/src/linphone_event.c])
AC_CONFIG_HEADERS([config.h])

PKG_CHECK_MODULES(ANTLR, [fremaks_utils >= 1.0])
PKG_CHECK_MODULES(FMS_UTILS, [fremaks_utils >= 1.0])
PKG_CHECK_MODULES(LINPHONE, [fremaks_utils >= 1.0])
PKG_CHECK_MODULES(BELLESIP, [fremaks_utils >= 1.0])
PKG_CHECK_MODULES(LIBXML2, [linphone >= 3.8.0])
# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC                   


AC_ARG_ENABLE(android,
        [AS_HELP_STRING([--enable-android=[yes/no]], [Turn on compilation of android support (default=no)])],
        [case "${enableval}" in
                yes)    enable_android=true ;;
                no)     enable_android=false ;;
                *)      AC_MSG_ERROR(bad value ${enableval} for --enable-android) ;;
        esac],
        [enable_android=false]
)
AM_CONDITIONAL(BUILD_ANDROID, test x$enable_android = xtrue)
if test x$enable_android = xtrue; then
        AC_DEFINE(ANDROID,1,[Tells tunnel extension is built-in])
fi

AC_OUTPUT(Makefile base/Makefile base/include/Makefile base/src/Makefile base/test/Makefile jni/Makefile)
