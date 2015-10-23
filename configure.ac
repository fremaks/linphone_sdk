#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([linphone_sdk], [1.0], [fremaks@163.com])
AM_INIT_AUTOMAKE([linphone_sdk], [1.0])
# Checks for programs.

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

#AC_CONFIG_SUBDIRS([alsa-lib-1.0.23
#                   belle-sip-1.4.1
#                   libantlr3c-3.4
#                   libxml2-2.9.1
#                   linphone-3.8.5
#                   linphone_interface])
AC_CONFIG_SUBDIRS([fremaks_utils
		   libantlr3c-3.4
		   belle-sip-1.4.1
                   libxml2-2.9.1
		   linphone-3.8.5
		   linphone_interface])
AC_ARG_ENABLE(android,
        [AS_HELP_STRING([--enable-android=[yes/no]], [Turn on compilation of android support (default=no)])],
        [case "${enableval}" in
                yes)    enable_android=true ;;
                no)     enable_android=false ;;
                *)      AC_MSG_ERROR(bad value ${enableval} for --enable-android) ;;
        esac],
        [enable_android=false]
)
AM_CONDITIONAL(BUILD_ALSA, test x$enable_android != xtrue)
if test "$enable_android" != "true"; then
AC_CONFIG_SUBDIRS([alsa-lib-1.0.23])
fi

AC_OUTPUT(Makefile)
