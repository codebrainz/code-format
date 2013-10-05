#ifndef FORMAT_H_
#define FORMAT_H_ 1

#include "plugin.h"

G_BEGIN_DECLS

/**
 * Wrapper around clang-format command-line utility.
 *
 * @param file_name The path to the file backing the document. This is
 * used to get the dirname() from and then change into that as the
 * working directory for clang-format so it finds the correct
 * .clang-format file.
 * @param code The source code (ie. document contents).
 * @param length The length in bytes of the @a code.
 * @param cursor Pointer to a variable containing the current cursor
 * position, the value will be updated with the new cursor position
 * upon return.
 * @param offset The start of the range to format.
 * @param length The length of the range to format.
 * @return A new GString containing the re-formated text or @c NULL on
 * error. The document's text should be replaced with this and then the
 * caret/cursor position should be updated from the value @a cursor
 * points to.
 */
GString *fmt_clang_format(const char *file_name, const char *code,
                          size_t code_len, size_t *cursor, size_t offset,
                          size_t length);

/**
 * Generates .clang-format contents based on an existing style.
 *
 * @param base_on_name The name of the style to base on.
 * @return The text containing the YAML configuration data.
 */
GString *fmt_clang_format_default_config(const char *based_on_name);

/**
 * Checks if @a path points to a an executable.
 *
 * It can either be absolute, relative or found in the `PATH`
 * environment variable.
 *
 * @param path The path to check.
 * @return @c true if the path is found, @c false otherwise.
 */
bool fmt_check_clang_format(const char *path);

char *fmt_lookup_clang_format_dot_file(const char *start_at);

bool fmt_can_find_clang_format_dot_file(const char *start_at);

G_END_DECLS

#endif // FORMAT_H_
