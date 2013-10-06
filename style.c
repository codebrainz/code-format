/*
 * style.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "style.h"

static struct
{
  const char *name;
  const char *label;
  const char *cmd_name;
} fmt_style_info[] = { { "custom", "Custom '.clang-format' File", "file" },
                       { "llvm", "LLVM", "LLVM" },
                       { "google", "Google", "Google" },
                       { "chromium", "Chromium", "Chromium" },
                       { "mozilla", "Mozilla", "Mozilla" },
                       { "webkit", "WebKit", "WebKit" },
                       // ...
};

size_t fmt_style_get_count(void)
{
  return G_N_ELEMENTS(fmt_style_info);
}

FmtStyle fmt_style_from_name(const char *name)
{
  for (size_t i = 0; i < G_N_ELEMENTS(fmt_style_info); i++)
  {
    if (g_ascii_strcasecmp(name, fmt_style_info[i].name) == 0)
      return (FmtStyle)i;
  }
  return FORMAT_STYLE_CUSTOM;
}

const char *fmt_style_get_label(FmtStyle style)
{
  return fmt_style_info[style].label;
}

const char *fmt_style_get_name(FmtStyle style)
{
  return fmt_style_info[style].name;
}

const char *fmt_style_get_cmd_name(FmtStyle style)
{
  return fmt_style_info[style].cmd_name;
}
