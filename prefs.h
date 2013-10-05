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

void fmt_prefs_save_panel(GtkWidget *panel, bool project);
GtkWidget *fmt_prefs_create_panel(bool project);

G_END_DECLS

#endif // FMT_PREFS_H
