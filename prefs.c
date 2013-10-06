/*
 * prefs.c
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

#include "prefs.h"
#include "format.h"

// Default config text directly included
#include "defconf.c"

#define PREF_GROUP "code-format"
#define PREF_PATH "clang-format-path"
#define PREF_STYLE "style"
#define PREF_AUTO "auto-format"
#define PREF_TRIGGER "auto-format-trigger-chars"
#define PREF_ONSAVE "format-on-save"

#define HAS_KEY(key) g_key_file_has_key(kf, PREF_GROUP, key, NULL)
#define GET_KEY(T, key) g_key_file_get_##T(kf, PREF_GROUP, key, NULL)
#define SET_KEY(T, k, v) g_key_file_set_##T(kf, PREF_GROUP, k, v)

struct FmtPreferences
{
  GString *path;
  FmtStyle style;
  bool auto_format;
  GString *trigger;
  bool on_save;
};

static struct FmtPreferences user_prefs;
static struct FmtPreferences proj_prefs;
static struct FmtPreferences *cur_prefs = NULL;

static void deinit_prefs(struct FmtPreferences *prefs)
{
  if (prefs->path)
  {
    g_string_free(prefs->path, true);
    prefs->path = NULL;
  }

  if (prefs->trigger)
  {
    g_string_free(prefs->trigger, true);
    prefs->trigger = NULL;
  }

  memset(prefs, 0, sizeof(struct FmtPreferences));
}

static void init_prefs(struct FmtPreferences *prefs)
{
  deinit_prefs(prefs);

  prefs->path = g_string_new("clang-format");
  prefs->style = FORMAT_STYLE_CUSTOM;
  prefs->auto_format = false;
  prefs->trigger = g_string_new(")}];");
  prefs->on_save = false;
}

static void clone_prefs(struct FmtPreferences *psrc,
                        struct FmtPreferences *pdst)
{
  pdst->style = psrc->style;
  pdst->auto_format = psrc->auto_format;
  g_string_assign(pdst->path, psrc->path->str);
  g_string_assign(pdst->trigger, psrc->trigger->str);
  pdst->on_save = psrc->on_save;
}

static void load_prefs(struct FmtPreferences *prefs, GKeyFile *kf)
{
  if (!g_key_file_has_group(kf, "code-format"))
    return;

  if (HAS_KEY("clang-format-path"))
  {
    char *val = GET_KEY(string, "clang-format-path");
    if (val)
    {
      g_string_assign(prefs->path, val);
      g_free(val);
    }
  }

  if (HAS_KEY("style"))
  {
    char *val = GET_KEY(string, "style");
    if (val)
    {
      prefs->style = fmt_style_from_name(val);
      g_free(val);
    }
  }

  if (HAS_KEY("auto-format"))
    prefs->auto_format = GET_KEY(boolean, "auto-format");

  if (HAS_KEY("auto-format-trigger-chars"))
  {
    char *val = GET_KEY(string, "auto-format-trigger-chars");
    if (val)
    {
      g_string_assign(prefs->trigger, val);
      g_free(val);
    }
  }

  if (HAS_KEY("format-on-save"))
    prefs->on_save = GET_KEY(boolean, "format-on-save");
}

static void open_user_prefs(void)
{
  char *fn, *dn;
  GKeyFile *kf;

  fn = g_build_filename(geany_data->app->configdir, "plugins",
                        "code-format.conf", NULL);
  dn = g_path_get_dirname(fn);

  g_mkdir_with_parents(dn, 0755);
  g_free(dn);

  if (!g_file_test(fn, G_FILE_TEST_EXISTS))
    g_file_set_contents(fn, fmt_def_conf, fmt_def_conf_length, NULL);

  kf = g_key_file_new();
  g_key_file_load_from_file(
      kf, fn, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);
  g_free(fn);

  load_prefs(&user_prefs, kf);

  g_key_file_free(kf);
}

static void save_prefs(struct FmtPreferences *prefs, GKeyFile *kf)
{
  SET_KEY(string, "clang-format-path", prefs->path->str);
  SET_KEY(string, "style", fmt_style_get_name(prefs->style));
  SET_KEY(boolean, "auto-format", prefs->auto_format);
  SET_KEY(string, "auto-format-trigger-chars", prefs->trigger->str);
  SET_KEY(boolean, "format-on-save", prefs->on_save);
}

void fmt_prefs_init(void)
{
  init_prefs(&user_prefs);
  init_prefs(&proj_prefs);
  open_user_prefs();
  cur_prefs = &user_prefs;
}

void fmt_prefs_deinit(void)
{
  deinit_prefs(&user_prefs);
  deinit_prefs(&proj_prefs);
  cur_prefs = &user_prefs;
}

void fmt_prefs_open_project(GKeyFile *kf)
{
  clone_prefs(&user_prefs, &proj_prefs); // base on user prefs
  load_prefs(&proj_prefs, kf);
  cur_prefs = &proj_prefs;
}

void fmt_prefs_close_project(void)
{

  init_prefs(&proj_prefs); // reset to defaults
  cur_prefs = &user_prefs;
}

void fmt_prefs_save_project(GKeyFile *kf)
{
  save_prefs(&proj_prefs, kf);
}

void fmt_prefs_save_user(void)
{
  GKeyFile *kf = g_key_file_new();
  char *contents;
  size_t length = 0;
  char *fn = g_build_filename(geany_data->app->configdir, "plugins",
                              "code-format.conf", NULL);

  // Load old contents in case user changed file outside of GUI
  g_key_file_load_from_file(
      kf, fn, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);

  // Update with new contents
  save_prefs(&user_prefs, kf);

  contents = g_key_file_to_data(kf, &length, NULL);
  if (contents)
  {
    // Store back on disk
    g_file_set_contents(fn, contents, length, NULL);
    g_free(contents);
  }

  g_free(fn);
  g_key_file_free(kf);
}

const char *fmt_prefs_get_path(void)
{
  return cur_prefs->path->str;
}

void fmt_prefs_set_path(const char *fn)
{
  g_string_assign(cur_prefs->path, fn);
}

FmtStyle fmt_prefs_get_style(void)
{
  return cur_prefs->style;
}

void fmt_prefs_set_style(FmtStyle style)
{
  cur_prefs->style = style;
}

bool fmt_prefs_get_auto_format(void)
{
  return cur_prefs->auto_format;
}

void fmt_prefs_set_auto_format(bool auto_format)
{
  cur_prefs->auto_format = auto_format;
}

const char *fmt_prefs_get_trigger(void)
{
  return cur_prefs->trigger->str;
}

void fmt_prefs_set_trigger(const char *trigger_chars)
{
  g_string_assign(cur_prefs->trigger, trigger_chars);
}

bool fmt_prefs_get_format_on_save(void)
{
  return cur_prefs->on_save;
}

void fmt_prefs_set_format_on_save(bool on_save)
{
  cur_prefs->on_save = on_save;
}

//======================================================================
//
// UI Stuff
//

#define UI_PATH PREF_GROUP "-" PREF_PATH
#define UI_STYLE PREF_GROUP "-" PREF_STYLE
#define UI_AUTO PREF_GROUP "-" PREF_AUTO
#define UI_TRIGGER PREF_GROUP "-" PREF_TRIGGER
#define UI_ON_SAVE PREF_GROUP "-" PREF_ONSAVE
#define UI_TRIG_LBL UI_TRIGGER "-label"
#define UI_TRIG_ENT UI_TRIGGER "-entry"
#define UI_CREATE UI_STYLE "-create-button"

#define GET_WIDGET(parent, name) g_object_get_data(G_OBJECT(parent), name)
#define SET_WIDGET(parent, name, wid) \
  g_object_set_data(G_OBJECT(parent), name, wid)

// Disable auto-format related options when auto-format is disabled
static void on_pref_auto_format_toggled(GtkToggleButton *btn,
                                        G_GNUC_UNUSED gpointer user_data)
{
  GtkWidget *lbl = GET_WIDGET(btn, UI_TRIG_LBL);
  GtkWidget *ent = GET_WIDGET(btn, UI_TRIG_ENT);
  // FIXME: why are sometimes NULL?
  if (GTK_IS_LABEL(lbl))
    gtk_widget_set_sensitive(lbl, gtk_toggle_button_get_active(btn));
  if (GTK_IS_ENTRY(ent))
    gtk_widget_set_sensitive(ent, gtk_toggle_button_get_active(btn));
}

// Disable Create button when Custom is selected
static void on_pref_style_changed(GtkComboBox *cb,
                                  G_GNUC_UNUSED gpointer user_data)
{
  FmtStyle style = (FmtStyle)gtk_combo_box_get_active(cb);
  GtkWidget *btn = GET_WIDGET(cb, UI_CREATE);
  gtk_widget_set_sensitive(btn, !(style == FORMAT_STYLE_CUSTOM));
}

// Create a .clang-format document based on selected style when Create is
// clicked
static void on_pref_create_clicked(GtkButton *button,
                                   G_GNUC_UNUSED gpointer user_data)
{
  GtkComboBox *combo = GET_WIDGET(button, UI_STYLE);
  FmtStyle style = (FmtStyle)gtk_combo_box_get_active(combo);
  const char *based_on = fmt_style_get_name(style);
  GString *str = fmt_clang_format_default_config(based_on);
  if (str)
  {
    GeanyDocument *doc = document_new_file(
        ".clang-format", filetypes[GEANY_FILETYPES_YAML], str->str);
    document_set_text_changed(doc, true);
    g_string_free(str, true);
    gtk_combo_box_set_active(combo, (int)FORMAT_STYLE_CUSTOM);
  }
}

// Update the icon in the path entry to show whether valid path is set
static void validate_clang_path_entry(GtkEntry *ent)
{
  const char *path = gtk_entry_get_text(ent);
  if (fmt_check_clang_format(path))
  {
    gtk_entry_set_icon_from_stock(ent, GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_OK);
  }
  else
  {
    gtk_entry_set_icon_from_stock(ent, GTK_ENTRY_ICON_SECONDARY,
                                  GTK_STOCK_DIALOG_ERROR);
  }
}

// Update the valid/invalid icon in the path entry when the text changes
static void on_pref_clang_format_path_changed(GtkEntry *ent,
                                              G_GNUC_UNUSED gpointer user_data)
{
  validate_clang_path_entry(ent);
}

void fmt_prefs_save_panel(GtkWidget *panel, bool project)
{
  GtkWidget *w_path, *w_style, *w_auto, *w_trigger, *w_onsave;
  struct FmtPreferences *p = NULL;

  if (project && geany_data->app->project)
    p = &proj_prefs;
  if (!p)
    p = &user_prefs;

  w_path = GET_WIDGET(panel, UI_PATH);
  w_style = GET_WIDGET(panel, UI_STYLE);
  w_auto = GET_WIDGET(panel, UI_AUTO);
  w_trigger = GET_WIDGET(panel, UI_TRIGGER);
  w_onsave = GET_WIDGET(panel, UI_ON_SAVE);

  g_string_assign(p->path, gtk_entry_get_text(GTK_ENTRY(w_path)));
  p->style = (FmtStyle)gtk_combo_box_get_active(GTK_COMBO_BOX(w_style));
  p->auto_format = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w_auto));
  g_string_assign(p->trigger, gtk_entry_get_text(GTK_ENTRY(w_trigger)));
  p->on_save = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w_onsave));

  if (p == &user_prefs)
    fmt_prefs_save_user();
  else
  {
    GKeyFile *kf = g_key_file_new();
    char *contents;
    size_t length = 0;
    // Load old contents
    g_key_file_load_from_file(
        kf, geany_data->app->project->file_name,
        G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, NULL);
    // Update with new contents
    fmt_prefs_save_project(kf);
    contents = g_key_file_to_data(kf, &length, NULL);
    if (contents)
    {
      // Store updated to disk
      g_file_set_contents(geany_data->app->project->file_name, contents, length,
                          NULL);
      g_free(contents);
    }
    g_key_file_free(kf);
  }
}

GtkWidget *fmt_prefs_create_panel(bool project)
{
  int row = 0;
  GtkWidget *grid, *lbl, *ent, *combo, *chk, *box, *btn, *sep;
  struct FmtPreferences *p = NULL;

  if (project && geany_data->app->project)
    p = &proj_prefs;
  if (!p)
    p = &user_prefs;

#if GTK_CHECK_VERSION(3, 0, 0)
  grid = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
#else
  grid = gtk_table_new(7, 3, false);
  gtk_table_set_col_spacings(GTK_TABLE(grid), 5);
  gtk_table_set_row_spacings(GTK_TABLE(grid), 5);
#endif
  gtk_container_set_border_width(GTK_CONTAINER(grid), 12);

  lbl = gtk_label_new(_("ClangFormat Path:"));
  gtk_misc_set_alignment(GTK_MISC(lbl), 0.0, 0.5);
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_grid_attach(GTK_GRID(grid), lbl, 0, row, 1, 1);
  gtk_widget_set_hexpand(lbl, false);
#else
  gtk_table_attach(GTK_TABLE(grid), lbl, 0, 1, row, row + 1, GTK_FILL, GTK_FILL,
                   0, 0);
#endif

  ent = gtk_entry_new();
  box = ui_path_box_new(_("Choose Path to 'clang-format'"),
                        GTK_FILE_CHOOSER_ACTION_OPEN, GTK_ENTRY(ent));
  gtk_entry_set_text(GTK_ENTRY(ent), p->path->str);
  gtk_entry_set_icon_activatable(GTK_ENTRY(ent), GTK_ENTRY_ICON_SECONDARY,
                                 false);
  validate_clang_path_entry(GTK_ENTRY(ent));
  g_signal_connect(ent, "changed",
                   G_CALLBACK(on_pref_clang_format_path_changed), NULL);
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_grid_attach(GTK_GRID(grid), box, 1, row, 2, 1);
  gtk_widget_set_hexpand(box, true);
#else
  gtk_table_attach(GTK_TABLE(grid), box, 1, 3, row, row + 1,
                   GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
#endif
  gtk_widget_set_tooltip_text(
      ent, _("Specify the path to the "
             "'clang-format' binary/executable on your system. If it's "
             "located in one of the directories found in the 'PATH' "
             "environment variable, you can just leave it as 'clang-format', "
             "otherwise you can specify the actual path to it. When you "
             "have a specified a valid 'clang-format' executable, an 'OK' "
             "icon will appear in the text box, otherwise if it's not a "
             "valid 'clang-format' executable, an 'error' icon will appear."));
  SET_WIDGET(grid, UI_PATH, ent);

  row++;

  lbl = gtk_label_new(_("Style:"));
  gtk_misc_set_alignment(GTK_MISC(lbl), 0.0, 0.5);
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_grid_attach(GTK_GRID(grid), lbl, 0, row, 1, 1);
  gtk_widget_set_hexpand(lbl, false);
#else
  gtk_table_attach(GTK_TABLE(grid), lbl, 0, 1, row, row + 1, GTK_FILL, GTK_FILL,
                   0, 0);
#endif

  combo = gtk_combo_box_text_new();
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_grid_attach(GTK_GRID(grid), combo, 1, row, 1, 1);
  gtk_widget_set_hexpand(combo, true);
#else
  gtk_table_attach(GTK_TABLE(grid), combo, 1, 2, row, row + 1,
                   GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
#endif
  gtk_widget_set_tooltip_text(
      combo, _("Select a preset code style "
               "from the list, or make your own using 'Custom'. See the "
               "help file for more information on custom code styles."));
  SET_WIDGET(grid, UI_STYLE, combo);

  // Add the style presets to the combo
  for (size_t i = 0; i < fmt_style_get_count(); i++)
  {
#if GTK_CHECK_VERSION(3, 0, 0)
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                              fmt_style_get_name((FmtStyle)i),
                              fmt_style_get_label((FmtStyle)i));
#else
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo),
                              fmt_style_get_label((FmtStyle)i));
#endif
  }

  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), (int)p->style);

  btn = gtk_button_new_with_label(_("Create"));
  SET_WIDGET(btn, UI_STYLE, combo);
  g_signal_connect(btn, "clicked", G_CALLBACK(on_pref_create_clicked), NULL);
  gtk_widget_set_sensitive(btn, false);
  SET_WIDGET(combo, UI_CREATE, btn);
  g_signal_connect(combo, "changed", G_CALLBACK(on_pref_style_changed), NULL);
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_grid_attach(GTK_GRID(grid), btn, 2, row, 1, 1);
  gtk_widget_set_hexpand(btn, false);
#else
  gtk_table_attach(GTK_TABLE(grid), btn, 2, 3, row, row + 1, GTK_FILL, GTK_FILL,
                   0, 0);
#endif
  gtk_widget_set_tooltip_text(
      btn, _("If you press this button, "
             "it will open up a new '.clang-format' file based on the "
             "preset style listed in the list. Save the file in the source "
             "code directory containing the files you want auto-formatting "
             "to affect."));

  row++;

  chk = gtk_check_button_new_with_label(_("Format documents when saving."));
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_grid_attach(GTK_GRID(grid), chk, 0, row, 3, 1);
  gtk_widget_set_hexpand(chk, true);
#else
  gtk_table_attach(GTK_TABLE(grid), chk, 0, 3, row, row + 1,
                   GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
#endif
  gtk_widget_set_tooltip_text(
      chk, _("Enabling this option causes "
             "the document to be formatted just before it is saved. This is "
             "especially useful if you aren't using auto-formatting."));
  SET_WIDGET(grid, UI_ON_SAVE, chk);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk), p->on_save);

  row++;

  chk = gtk_check_button_new_with_label(_("Enable auto-formatting"));
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_grid_attach(GTK_GRID(grid), chk, 0, row, 3, 1);
  gtk_widget_set_hexpand(chk, true);
#else
  gtk_table_attach(GTK_TABLE(grid), chk, 0, 3, row, row + 1,
                   GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
#endif
  g_signal_connect(chk, "toggled", G_CALLBACK(on_pref_auto_format_toggled),
                   NULL);
  gtk_widget_set_tooltip_text(
      chk, _("When auto-formatting is enabled, "
             "a special set of 'trigger' characters can be specified to cause "
             "the document to be re-formatted. This is useful if you want to "
             "keep the code formatted as you type without actually worrying "
             "about formatting at all."));
  SET_WIDGET(grid, UI_AUTO, chk);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk), p->auto_format);

  row++;

  lbl = gtk_label_new(_("Trigger Chars:"));
  gtk_widget_set_sensitive(lbl, p->auto_format);
  SET_WIDGET(chk, UI_TRIG_LBL, lbl);
  gtk_misc_set_alignment(GTK_MISC(lbl), 0.0, 0.5);
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_grid_attach(GTK_GRID(grid), lbl, 0, row, 1, 1);
  gtk_widget_set_hexpand(lbl, false);
#else
  gtk_table_attach(GTK_TABLE(grid), lbl, 0, 1, row, row + 1, GTK_FILL, GTK_FILL,
                   0, 0);
#endif

  ent = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(ent), p->trigger->str);
  gtk_widget_set_sensitive(ent, p->auto_format);
  SET_WIDGET(chk, UI_TRIG_ENT, ent);
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_grid_attach(GTK_GRID(grid), ent, 1, row, 2, 1);
  gtk_widget_set_hexpand(ent, true);
#else
  gtk_table_attach(GTK_TABLE(grid), ent, 1, 3, row, row + 1,
                   GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
#endif
  gtk_widget_set_tooltip_text(
      ent, _("When auto-formatting is "
             "enabled, these characters are used to trigger the "
             "auto-formatting to occur. The default set seems to work "
             "well (ie. ')}];'). Note: do not use '\\n', it doesn't "
             "work."));
  SET_WIDGET(grid, UI_TRIGGER, ent);

  row++;

#if GTK_CHECK_VERSION(3, 0, 0)
  sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_grid_attach(GTK_GRID(grid), sep, 0, row, 3, 1);
#else
  sep = gtk_hseparator_new();
  gtk_table_attach(GTK_TABLE(grid), sep, 0, 3, row, row + 1,
                   GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
#endif

  row++;

  lbl = gtk_link_button_new_with_label(
      "http://clang.llvm.org/docs/ClangFormat.html",
      _("Click here for more information about ClangFormat"));
#if GTK_CHECK_VERSION(3, 0, 0)
  gtk_grid_attach(GTK_GRID(grid), lbl, 0, row, 3, 1);
#else
  gtk_table_attach(GTK_TABLE(grid), lbl, 0, 3, row, row + 1,
                   GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
#endif

  return grid;
}
