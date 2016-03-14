AC_INIT([geany-code-format], [0.01], [matt@geany.org])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([plugin.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])
AM_PROG_AR
LT_INIT([disable-static pic-only])
AC_PROG_CC_C99
PKG_CHECK_MODULES([GEANY], [geany >= 1.23])
AC_CONFIG_FILES([Makefile compile_commands.json])
AC_OUTPUT
