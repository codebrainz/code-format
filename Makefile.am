ACLOCAL_AMFLAGS = -I m4

geany_plugindir = $(libdir)/geany
geany_plugin_LTLIBRARIES = codeformat.la

codeformat_la_CFLAGS = $(GEANY_CFLAGS) -DG_LOG_DOMAIN=\""CodeFormat"\"
codeformat_la_LIBADD = $(GEANY_LIBS)
codeformat_la_LDFLAGS = -module -avoid-version
codeformat_la_SOURCES = \
	format.c format.h \
	plugin.c plugin.h \
	prefs.c prefs.h \
	process.c process.h \
	replacements.c replacements.h \
	style.c style.h

cf_docdir = $(docdir)/geany-code-format
cf_doc_DATA = README.md

EXTRA_DIST = defconf.c
