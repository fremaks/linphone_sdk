#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(fremaks_utils, 1.0.0, [fremaks@163.com])
AM_INIT_AUTOMAKE(fremaks_utils, 1.0.0)
AC_CONFIG_SRCDIR([fms_common/src/fms_file.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
#AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset mkdir rmdir strrchr])


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
        AC_DEFINE(OS_ANDROID,1,[Tells tunnel extension is built-in])
fi

AC_OUTPUT(Makefile include/Makefile fms_common/Makefile fms_common/include/Makefile fms_common/src/Makefile fms_common/test/Makefile)
