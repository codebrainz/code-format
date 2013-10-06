#!/usr/bin/env make

ifndef DEST
DEST=~/.config/geany
endif

fmt_CFLAGS  := $(strip $(CFLAGS) -g -Wall -Wextra -Werror -Wno-unused-parameter -std=c99 $(shell pkg-config --cflags geany))
fmt_LDFLAGS := $(strip $(LDFLAGS) $(shell pkg-config --libs geany))

all: format.so

format.so: plugin.o format.o style.o prefs.o replacements.o
	$(CC) -shared $(fmt_CFLAGS) -o $@ $^ $(fmt_LDFLAGS)

replacements.o: replacements.c
	$(CC) -c -fPIC $(fmt_CFLAGS) -o $@ $<

plugin.o: plugin.c
	$(CC) -c -fPIC $(fmt_CFLAGS) -o $@ $<

format.o: format.c format.h
	$(CC) -c -fPIC $(fmt_CFLAGS) -o $@ $<

style.o: style.c style.h
	$(CC) -c -fPIC $(fmt_CFLAGS) -o $@ $<

prefs.o: prefs.c prefs.h
	$(CC) -c -fPIC $(fmt_CFLAGS) -o $@ $<

install: format.so
	mkdir -p $(DEST)/plugins/
	cp $< $(DEST)/plugins/

uninstall:
	$(RM) $(DEST)/plugins/format.so

clean:
	$(RM) *.o *.so

.PHONY: all install clean
