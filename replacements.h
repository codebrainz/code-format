#ifndef FMT_REPLACEMENTS_H
#define FMT_REPLACEMENTS_H

#include "plugin.h"

G_BEGIN_DECLS

typedef struct
{
  size_t offset;
  size_t length;
  GString *repl_text;
} Replacement;

Replacement *replacement_new(size_t offset, size_t length,
                             const char *repl_text);

void replacement_free(Replacement *repl);

GPtrArray *replacements_parse(GString *xml);

G_END_DECLS

#endif // FMT_REPLACEMENTS_H
