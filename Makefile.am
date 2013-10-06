ACLOCAL_AMFLAGS = -I m4

cf_docdir = $(docdir)/geany-code-format
cf_doc_DATA = README.md

geany_plugindir = $(libdir)/geany
geany_plugin_LTLIBRARIES = codeformat.la

codeformat_la_CFLAGS = $(GEANY_CFLAGS) \
	-DG_LOG_DOMAIN=\""CodeFormat"\" \
	-DFMT_README_FILE=\""$(cf_docdir)/README.md"\"
codeformat_la_LIBADD = $(GEANY_LIBS)
codeformat_la_LDFLAGS = -module -avoid-version
codeformat_la_SOURCES = \
	format.c format.h \
	plugin.c plugin.h \
	prefs.c prefs.h \
	process.c process.h \
	style.c style.h

EXTRA_DIST = defconf.c
