/*
 * prefs.h
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

#ifndef FMT_PREFS_H
#define FMT_PREFS_H

#include "style.h"
#include "plugin.h"

G_BEGIN_DECLS

void fmt_prefs_init(void);
void fmt_prefs_deinit(void);

void fmt_prefs_open_project(GKeyFile *kf);
void fmt_prefs_close_project(void);
void fmt_prefs_save_project(GKeyFile *kf);
void fmt_prefs_save_user(void);

const char *fmt_prefs_get_path(void);
void fmt_prefs_set_path(const char *fn);
FmtStyle fmt_prefs_get_style(void);
void fmt_prefs_set_style(FmtStyle style);
bool fmt_prefs_get_auto_format(void);
void fmt_prefs_set_auto_format(bool auto_format);
const char *fmt_prefs_get_trigger(void);
void fmt_prefs_set_trigger(const char *trigger_chars);
bool fmt_prefs_get_format_on_save(void);
void fmt_prefs_set_format_on_save(bool on_save);

void fmt_prefs_save_panel(GtkWidget *panel, bool project);
GtkWidget *fmt_prefs_create_panel(bool project);

G_END_DECLS

#endif // FMT_PREFS_H
