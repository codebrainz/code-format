/*
 * style.h
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

#ifndef FORMAT_STYLE_H
#define FORMAT_STYLE_H

#include "plugin.h"

G_BEGIN_DECLS

typedef enum
{
  FORMAT_STYLE_CUSTOM = 0,
  FORMAT_STYLE_LLVM,
  FORMAT_STYLE_GOOGLE,
  FORMAT_STYLE_CHROMIUM,
  FORMAT_STYLE_MOZILLA,
  FORMAT_STYLE_WEBKIT,
} FmtStyle;

size_t fmt_style_get_count(void);
FmtStyle fmt_style_from_name(const char *name);
const char *fmt_style_get_label(FmtStyle style);
const char *fmt_style_get_name(FmtStyle style);
const char *fmt_style_get_cmd_name(FmtStyle style);

G_END_DECLS

#endif // FORMAT_STYLE_H
