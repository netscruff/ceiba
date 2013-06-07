#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ceiba], [1.0], [sanderm@email.arizona.edu])
AC_CONFIG_SRCDIR([src/aa.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Include local macros
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_LEX
LT_INIT
AC_PROG_YACC
AX_CHECK_GL
AX_CHECK_GLU
AX_CHECK_GLUT

# Checks for libraries.
# FIXME: Replace `main' with a function in `-ll':
AC_CHECK_LIB([l], [main])
# FIXME: Replace `main' with a function in `-lmy_structures':
AC_CHECK_LIB([my_structures], [main])
AC_CHECK_LIB([freetype], [FT_Init_FreeType], LIBS="$LIBS -lfreetype")
AC_CHECK_LIB([GLE], [gleGetJoinStyle],
	LIBS="$LIBS -lgle")

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([inttypes.h libintl.h malloc.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor memset pow sqrt strdup strrchr strstr strtol])

# AC_CONFIG_FILES([src/my_structures/makefile])
AC_OUTPUT(Makefile src/Makefile)

