/*
 * format.c
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

#include "format.h"
#include "style.h"
#include "prefs.h"
#include "process.h"

extern GeanyFunctions *geany_functions;

static GPtrArray *format_arguments(size_t cursor, size_t offset, size_t length,
                                   bool xml_replacements)
{
  GPtrArray *args = g_ptr_array_new_with_free_func(g_free);
  const char *path;

  path = fmt_prefs_get_path();
  if (path && *path)
    g_ptr_array_add(args, g_strdup(path));
  else
    g_ptr_array_add(args, g_strdup("clang-format"));

  if (xml_replacements)
    g_ptr_array_add(args, g_strdup("-output-replacements-xml"));

  g_ptr_array_add(
      args, g_strdup_printf("-style=%s",
                            fmt_style_get_cmd_name(fmt_prefs_get_style())));

  g_ptr_array_add(args, g_strdup_printf("-cursor=%lu", cursor));
  g_ptr_array_add(args, g_strdup_printf("-offset=%lu", offset));
  g_ptr_array_add(args, g_strdup_printf("-length=%lu", length));
  g_ptr_array_add(args, NULL);

  return args;
}

#define INVALID_CURSOR ((size_t) - 1)

static size_t extract_cursor(GString *str)
{
  char *first_nl, *first_line = NULL, *it;
  char num_buf[64] = { 0 };
  const char *nl_chars;
  size_t first_len, cnt, cursor_pos;

  nl_chars = "\r\n";
  first_nl = strstr(str->str, "\r\n");
  if (!first_nl)
  {
    nl_chars = "\n";
    first_nl = strchr(str->str, '\n');
    if (!first_nl)
    {
      nl_chars = "\r";
      first_nl = strchr(str->str, '\r');
      if (!first_nl)
        return INVALID_CURSOR;
    }
  }

  first_len = first_nl - str->str;
  first_line = g_strndup(str->str, first_len);
  if (!first_line)
    return INVALID_CURSOR;

  // Remove the first line containing the cursor position
  g_string_erase(str, 0, first_len + strlen(nl_chars));

  // Sample: { "Cursor": 4 }
  it = strstr(first_line, "\"Cursor\":");
  if (!it)
  {
    g_free(first_line);
    return INVALID_CURSOR;
  }
  it += 9; // "Cursor":

  for (cnt = 0; cnt < 64 && it && *it && (isspace(*it) || isdigit(*it));
       cnt++, it++)
  {
    num_buf[cnt] = *it;
  }
  g_strstrip(num_buf);

  g_free(first_line);

  errno = 0;
  cursor_pos = strtoul(num_buf, NULL, 10);
  if (errno != 0)
    return INVALID_CURSOR;

  return cursor_pos;
}

GString *fmt_clang_format(const char *file_name, const char *code,
                          size_t code_len, size_t *cursor, size_t offset,
                          size_t length, bool xml_replacements)
{
  char *work_dir;
  GPtrArray *args;
  GString *out;
  size_t cursor_pos;
  FmtProcess *proc;

  g_return_val_if_fail(file_name, NULL);
  g_return_val_if_fail(code, NULL);
  g_return_val_if_fail(code_len, NULL);
  g_return_val_if_fail(cursor, NULL);
  g_return_val_if_fail(length, NULL);

  args = format_arguments(*cursor, offset, length, xml_replacements);
  work_dir = g_path_get_dirname(file_name);

  proc = fmt_process_open(work_dir, (const char * const *)args->pdata);

  g_ptr_array_free(args, TRUE);
  g_free(work_dir);

  if (!proc) // In case clang-format cannot be found
    return NULL;

  out = g_string_sized_new(code_len);
  if (!fmt_process_run(proc, code, code_len, out))
  {
    g_warning("Failed to format document range");
    g_string_free(out, true);
    fmt_process_close(proc);
    return NULL;
  }

// FIXME: clang-format returns non-zero when it can't find the
// .clang-format file, handle this case specially
#if 1
  fmt_process_close(proc);
#else
  if (fmt_process_close(proc) != 0)
  {
    g_warning("Subprocess returned non-zero exit code");
    g_string_free(out, true);
    return NULL;
  }
#endif

  if (!xml_replacements)
  {
    cursor_pos = extract_cursor(out);
    if (cursor_pos == INVALID_CURSOR)
    {
      g_warning(
          "Failed to parse resulting cursor position from resulting code");
      g_string_free(out, true);
      return NULL;
    }
    *cursor = cursor_pos;
  }

  return out;
}

GString *fmt_clang_format_default_config(const char *based_on_name)
{
  GString *str;
  GPtrArray *args = g_ptr_array_new_with_free_func(g_free);
  FmtProcess *proc;
  const char *path;

  path = fmt_prefs_get_path();

  if (path && *path)
    g_ptr_array_add(args, g_strdup(path));
  else
    g_ptr_array_add(args, g_strdup("clang-format"));

  g_ptr_array_add(
      args, g_strdup_printf(
                "-style=%s",
                fmt_style_get_cmd_name(fmt_style_from_name(based_on_name))));
  g_ptr_array_add(args, g_strdup("-dump-config"));
  g_ptr_array_add(args, NULL);

  proc = fmt_process_open(NULL, (const char * const *)args->pdata);
  g_ptr_array_free(args, true);

  if (!proc)
    return NULL;

  str = g_string_sized_new(1024);
  if (!fmt_process_run(proc, NULL, 0, str))
  {
    g_string_free(str, true);
    fmt_process_close(proc);
    return NULL;
  }
  fmt_process_close(proc);

  return str;
}

bool fmt_check_clang_format(const char *path)
{
  char *full_path;

  full_path = g_find_program_in_path(path);
  if (!full_path)
    return false;

  g_free(full_path);
  return true;
}

char *fmt_lookup_clang_format_dot_file(const char *start_at)
{
  char *fn, *dn, *last_fn;

  // NULL, "." or "" (empty) means use current directory
  if (start_at == NULL || start_at[0] == '\0' ||
      (start_at[0] == '.' && start_at[1] == '\0'))
  {
    char *cur = g_get_current_dir();
    dn = tm_get_real_path(cur);
    g_free(cur);
  }
  // Otherwise, if it's a file, get the dir name, if not use it
  else
  {
    char *real_start = tm_get_real_path(start_at);
    if (g_file_test(start_at, G_FILE_TEST_IS_DIR))
      dn = real_start;
    else
    {
      dn = g_path_get_dirname(real_start);
      g_free(real_start);
    }
  }

  if (!g_file_test(dn, G_FILE_TEST_EXISTS))
  {
    g_free(dn);
    return NULL;
  }

  // Walk backwards from dn
  fn = NULL;
  last_fn = NULL;
  while (fn == NULL)
  {
    fn = g_build_filename(dn, ".clang-format", NULL);
    if (fn)
    {
      // Bail out when top is reached
      if (last_fn && g_strcmp0(last_fn, fn) == 0)
      {
        g_free(fn);
        fn = NULL;
        break;
      }
      else if (g_file_test(fn, G_FILE_TEST_EXISTS))
      {
        break;
      }
      else
      {
        g_free(last_fn);
        last_fn = fn;
        fn = dn; // use as temp var
        dn = g_path_get_dirname(dn);
        g_free(fn); // stop using as temp va
        fn = NULL;
        if (!g_file_test(dn, G_FILE_TEST_EXISTS))
          break;
      }
    }
    else
    {
      break;
    }
  }
  g_free(dn);
  g_free(last_fn);

  return fn;
}

bool fmt_can_find_clang_format_dot_file(const char *start_at)
{
  char *fn = fmt_lookup_clang_format_dot_file(start_at);
  if (fn)
  {
    g_free(fn);
    return true;
  }
  return false;
}
