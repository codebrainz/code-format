AC_DEFUN([AX_CHECK_GEANY_GTK3], [
  PKG_CHECK_MODULES([GEANY], [geany > 1.23], [
    AC_MSG_CHECKING([check for GTK+ 3 Geany])
    have_geany_gtk3=`pkg-config --print-requires geany | grep 'gtk+-3.0' >/dev/null 2>&1; echo $?`
    if test $have_geany_gtk3 -ne 0
    then
      AC_MSG_ERROR([Failed to locate a GTK+ 3 build of Geany. \
Please make sure you have one installed and your pkg-config flags are setup accordingly.])
    fi
    AC_MSG_RESULT([yes])
  ])
])
