#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(macierze, 1.0, piotr.turu.turek@gmail.com)
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])
AM_PROG_CC_C_O
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_MSG_CHECKING([Should file IO be used?])
AC_ARG_ENABLE(
	fileoutput,
	AC_HELP_STRING([--enable-fileoutput], [Use file IO]),
	[ if test "x$enable_fileoutput" = "xyes" ; then
        	AC_DEFINE([WEWY_PLIKI], [], [Set if file io is used])
         	AC_MSG_RESULT([Yes])
      	else
         	AC_MSG_RESULT([No])
      	fi ],
	AC_MSG_RESULT([No])
)

AC_MSG_CHECKING([Enable logging?])
AC_ARG_ENABLE(
	logs,
	AC_HELP_STRING([--enable-logs], [Logs will be produced]),
	[ if test "x$enable_logs" = "xyes" ; then
	AC_DEFINE([KOMUNIKATY], [], [Set if logging enabled])
        	AC_MSG_RESULT([Yes])
      	else
         	AC_MSG_RESULT([No])
      	fi ],
	AC_MSG_RESULT([No])
)

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/include/Makefile
                 src/operacje/Makefile
                 src/reszta/Makefile
                 src/we_wy/Makefile])
AC_OUTPUT
