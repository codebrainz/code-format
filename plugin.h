#ifndef FMT_PLUGIN_H
#define FMT_PLUGIN_H

#include <glib.h>
#include <gtk/gtk.h>
#include <geanyplugin.h>

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS

extern GeanyPlugin *geany_plugin;
extern GeanyData *geany_data;
extern GeanyFunctions *geany_functions;

G_END_DECLS

#endif // FMT_PLUGIN_H
