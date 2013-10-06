#!/bin/sh
# Deletes a bunch of junk put in the tree by Auto-tools

rm -f *.o *.a *.so *.dll *.lib *.dylib *.lo *.la
rm -rf .deps/ .libs/ autom4te.cache/ build-aux/
rm -f m4/libtool.m4 m4/lt*.m4 Makefile Makefile.in libtool
rm -f config.* configure stamp-h1 aclocal.m4
rm -f compile_commands.json
