#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([zad1], [1.0], [pturek@student.agh.edu.pl])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([zad1.c])
AC_CONFIG_HEADERS([config.h])
AM_PROG_CC_C_O

AC_ARG_WITH(
	handler, 
	AC_HELP_STRING([--with-handler], [Set if SIGACTION is going to be used]),
	[ if test "x$with_handler" = "xsigaction" ; then
		 AC_DEFINE([SIGACTION], [], [Set if SIGACTION is going to be used])
         AC_MSG_RESULT([Yes])
      else
         AC_MSG_RESULT([No])
      fi ],
	AC_MSG_RESULT([No])
)



# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
