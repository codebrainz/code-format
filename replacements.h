/*
 * replacements.h
 *
 * Copyright 2013 Matthew <mbrush@codebrainz.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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
