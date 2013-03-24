#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT(mymat, 1.0, piotr.turu.turek@gmail.com)
AC_CONFIG_SRCDIR([src/mymat.c])
AC_CONFIG_HEADERS([config.h])
AM_PROG_CC_C_O
AM_INIT_AUTOMAKE
LT_INIT

# Checks for programs.
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC

AC_ARG_WITH(
	mem-size, 
	AC_HELP_STRING([--with-mem-size], [Ustaw rozmiar, wyrazony w blokach pamieci biblioteki mymem, pamieci dostepnej dla biblioteki]), 
	[ if test "x$with_mem_size" = "" ; then
         AC_MSG_RESULT([No])
         AC_DEFINE_UNQUOTED([MEM_SIZE], [100], [Rozmiar, wyrazony w blokach pamieci biblioteki mymem, pamieci dostepnej dla biblioteki])
      else
         AC_DEFINE_UNQUOTED([MEM_SIZE], [$with_mem_size], [Rozmiar, wyrazony w blokach pamieci biblioteki mymem, pamieci dostepnej dla biblioteki])
         AC_MSG_RESULT([Yes])
      fi ],	
    [ AC_DEFINE_UNQUOTED([MEM_SIZE], [100], [Rozmiar, wyrazony w blokach pamieci biblioteki mymem, pamieci dostepnej dla biblioteki])
	AC_MSG_RESULT([No]) ]
)

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/include/Makefile])
AC_OUTPUT
