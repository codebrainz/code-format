#!/usr/bin/env bash
# This is a script I use to clean-out and completely reconfigure
# my Code Format tree. It has in the PREFIX location, the path
# to where Geany GTK3 is installed.

PREFIX=$HOME/.local

make clean
make distclean
./autogen.sh && \
  PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig" \
    ./configure --prefix="${PREFIX}"
