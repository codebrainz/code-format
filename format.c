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

#include "format.h"
#include "style.h"
#include "prefs.h"

#define IO_BUF_SIZE 4096

extern GeanyFunctions *geany_functions;

typedef struct
{
  int child_pid;
  GIOChannel *ch_in, *ch_out;
  int return_code;
  unsigned long exit_handler;
} Process;

static void on_process_exited(GPid pid, int status, Process *proc)
{
  g_spawn_close_pid(pid);
  proc->child_pid = 0;

  // FIXME: is it automatically removed?
  if (proc->exit_handler > 0)
  {
    g_source_remove(proc->exit_handler);
    proc->exit_handler = 0;
  }

  if (proc->ch_in)
  {
    g_io_channel_shutdown(proc->ch_in, true, NULL);
    g_io_channel_unref(proc->ch_in);
    proc->ch_in = NULL;
  }

  if (proc->ch_out)
  {
    g_io_channel_shutdown(proc->ch_out, true, NULL);
    g_io_channel_unref(proc->ch_out);
    proc->ch_out = NULL;
  }

  proc->return_code = status;
}

static Process *create_subprocess(const char *work_dir, const char *const *argv)
{
  Process *proc;
  GError *error = NULL;
  int fd_in = -1, fd_out = -1;

  proc = g_new0(Process, 1);

  if (!g_spawn_async_with_pipes(work_dir, (char **)argv, NULL,
                                G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                                NULL, NULL, &proc->child_pid, &fd_in, &fd_out,
                                NULL, &error))
  {
    g_warning("Failed to create subprocess: %s", error->message);
    g_error_free(error);
    g_free(proc);
    return NULL;
  }

  proc->return_code = -1;
  proc->exit_handler = g_child_watch_add(
      proc->child_pid, (GChildWatchFunc)on_process_exited, proc);

  // TODO: handle windows
  proc->ch_in = g_io_channel_unix_new(fd_in);
  proc->ch_out = g_io_channel_unix_new(fd_out);

  return proc;
}

static int close_subprocess(Process *proc)
{
  int ret_code = proc->return_code;

  if (proc->ch_in)
  {
    g_io_channel_shutdown(proc->ch_in, true, NULL);
    g_io_channel_unref(proc->ch_in);
  }

  if (proc->ch_out)
  {
    g_io_channel_shutdown(proc->ch_out, true, NULL);
    g_io_channel_unref(proc->ch_out);
  }

  if (proc->child_pid > 0)
  {
    if (proc->exit_handler > 0)
      g_source_remove(proc->exit_handler);
    g_spawn_close_pid(proc->child_pid);
  }

  g_free(proc);

  return ret_code;
}

static GPtrArray *format_arguments(size_t cursor, size_t offset, size_t length,
                                   bool xml_replacements)
{
  GPtrArray *args = g_ptr_array_new_full(5, g_free);
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

static bool run_process(Process *proc, const char *str_in, size_t in_len,
                        GString *str_out)
{
  GIOStatus status;
  GError *error = NULL;
  bool read_complete = false;
  size_t in_off = 0;

  if (str_in && in_len)
  {

    do
    { // until all text is pushed into process's stdin

      size_t bytes_written = 0;
      size_t write_size_remaining = in_len - in_off;
      size_t size_to_write = MIN(write_size_remaining, IO_BUF_SIZE);

      // Write some data to process's stdin
      error = NULL;
      status = g_io_channel_write_chars(proc->ch_in, str_in + in_off,
                                        size_to_write, &bytes_written, &error);

      in_off += bytes_written;

      if (status == G_IO_STATUS_ERROR)
      {
        g_warning("Failed writing to subprocess's stdin: %s", error->message);
        g_error_free(error);
        return false;
      }

    } while (in_off < in_len);
  }

  // Flush it and close it down
  g_io_channel_shutdown(proc->ch_in, true, NULL);
  g_io_channel_unref(proc->ch_in);
  proc->ch_in = NULL;

  // All text should be written to process's stdin by now, read the
  // rest of the process's stdout.
  while (!read_complete)
  {
    char *tail_string = NULL;
    size_t tail_len = 0;

    error = NULL;
    status =
        g_io_channel_read_to_end(proc->ch_out, &tail_string, &tail_len, &error);

    if (tail_len > 0)
      g_string_append_len(str_out, tail_string, tail_len);

    g_free(tail_string);

    if (status == G_IO_STATUS_ERROR)
    {
      g_warning("Failed to read rest of subprocess's stdout: %s",
                error->message);
      g_error_free(error);
      return false;
    }
    else if (status == G_IO_STATUS_AGAIN)
    {
      continue;
    }
    else
    {
      read_complete = true;
    }
  }

  return true;
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
  Process *proc;

  g_return_val_if_fail(file_name, NULL);
  g_return_val_if_fail(code, NULL);
  g_return_val_if_fail(code_len, NULL);
  g_return_val_if_fail(cursor, NULL);
  g_return_val_if_fail(length, NULL);

  args = format_arguments(*cursor, offset, length, xml_replacements);
  work_dir = g_path_get_dirname(file_name);

  proc = create_subprocess(work_dir, (const char * const *)args->pdata);

  g_ptr_array_free(args, TRUE);
  g_free(work_dir);

  if (!proc) // In case clang-format cannot be found
    return NULL;

  out = g_string_sized_new(code_len);
  if (!run_process(proc, code, code_len, out))
  {
    g_warning("Failed to format document range");
    g_string_free(out, true);
    close_subprocess(proc);
    return NULL;
  }

// FIXME: clang-format returns non-zero when it can't find the
// .clang-format file, handle this case specially
#if 1
  close_subprocess(proc);
#else
  if (close_subprocess(proc) != 0)
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
  Process *proc;
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

  proc = create_subprocess(NULL, (const char * const *)args->pdata);
  g_ptr_array_free(args, true);

  if (!proc)
    return NULL;

  str = g_string_sized_new(1024);
  if (!run_process(proc, NULL, 0, str))
  {
    g_string_free(str, true);
    close_subprocess(proc);
    return NULL;
  }
  close_subprocess(proc);

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

#ifdef IS_THIS_LIKE_SUPER_DANGEROUS
  // It would be run each character typed into the path box, which
  // means if you wanted to enter the name "rmanager" for a ficticious
  // program, you would first type "rm" and that would be executed. The
  // only question is whether having specific args to clang-format makes
  // it safe enough or just all around stupid? I'll leave it disabled
  // until I'm sure it's safe, for now just stick with a path lookup
  // and check for executable permissions.

  GString *str;
  GPtrArray *args;
  Process *proc;
  bool result = false;

  args = g_ptr_array_new_with_free_func(g_free);
  g_ptr_array_add(args, full_path);
  g_ptr_array_add(args, g_strdup("-style=LLVM"));
  g_ptr_array_add(args, g_strdup("-dump-config"));
  g_ptr_array_add(args, NULL);

  proc = create_subprocess(NULL, (const char * const *)args->pdata);
  g_ptr_array_free(args, true);

  if (!proc)
    return false;

  str = g_string_sized_new(1024);
  if (!run_process(proc, NULL, 0, str))
  {
    g_string_free(str, true);
    close_subprocess(proc);
    return false;
  }
  close_subprocess(proc);

  result = (str && str->len > 0);
  g_string_free(str, true);

  return result;
#endif
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
