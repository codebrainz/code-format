#include "replacements.h"

typedef struct
{
  bool in_repl;
  GQueue *stack;
  GPtrArray *nodes;
} ParseContext;

static ParseContext *parse_context_new(void)
{
  ParseContext *ctx = g_new0(ParseContext, 1);
  ctx->in_repl = false;
  ctx->stack = g_queue_new();
  ctx->nodes = g_ptr_array_new_with_free_func((GDestroyNotify)replacement_free);
  return ctx;
}

static void parse_context_free(ParseContext *ctx)
{
  if (!ctx)
    return;
  if (ctx->stack)
    g_queue_free_full(ctx->stack, (GDestroyNotify)replacement_free);
  if (ctx->nodes)
    g_ptr_array_free(ctx->nodes, true);
  g_free(ctx);
}

Replacement *replacement_new(size_t offset, size_t length,
                             const char *repl_text)
{
  Replacement *repl;
  g_return_val_if_fail(length > 0, NULL);
  g_return_val_if_fail(repl_text, NULL);
  repl = g_new0(Replacement, 1);
  repl->offset = offset;
  repl->length = length;
  repl->repl_text = g_string_new(repl_text);
  return repl;
}

void replacement_free(Replacement *repl)
{
  if (!repl)
    return;
  g_string_free(repl->repl_text, true);
  g_free(repl);
}

static void on_start_element(GMarkupParseContext *context,
                             const char *element_name,
                             const char **attribute_names,
                             const char **attribute_values, gpointer user_data,
                             GError **error)
{
  size_t i, nattrs;
  long offset = -1, len = -1;
  Replacement *repl;
  ParseContext *ctx = user_data;

  if (g_strcmp0(element_name, "replacement") != 0)
    return;

  if (!attribute_names)
    return;

  nattrs = g_strv_length((char **)attribute_names);
  for (i = 0; i < nattrs; i++) {
    if (g_strcmp0(attribute_names[i], "offset") == 0) {
      errno = 0;
      offset = strtol(attribute_values[i], NULL, 10);
      if (errno != 0) {
        offset = -1;
      }
    } else if (g_strcmp0(attribute_names[i], "length") == 0) {
      errno = 0;
      len = strtol(attribute_values[i], NULL, 10);
      if (errno != 0) {
        len = -1;
      }
    }
  }

  if (offset == -1 || len == -1)
    return;

  repl = replacement_new(offset, len, "");
  g_queue_push_head(ctx->stack, repl);
  ctx->in_repl = true;
}

static void on_end_element(GMarkupParseContext *context,
                           const char *element_name, gpointer user_data,
                           GError **error)
{
  ParseContext *ctx = user_data;
  Replacement *repl;
  if (g_queue_is_empty(ctx->stack))
    return;
  if (g_strcmp0(element_name, "replacement") != 0)
    return;
  repl = g_queue_pop_head(ctx->stack);
  ctx->in_repl = false;
  if (repl) {
    if (repl->repl_text->len == 0) // replacement text must be at least 1
      replacement_free(repl);
    else
      g_ptr_array_add(ctx->nodes, repl);
  }
}

static void on_text(GMarkupParseContext *context, const char *text,
                    size_t text_len, gpointer user_data, GError **error)
{
  ParseContext *ctx = user_data;
  Replacement *repl;
  if (!ctx->in_repl)
    return;
  repl = g_queue_peek_head(ctx->stack);
  if (repl)
    g_string_append_len(repl->repl_text, text, text_len);
}

static GMarkupParser parser_funcs = { on_start_element, on_end_element, on_text,
                                      NULL,             NULL };

GPtrArray *replacements_parse(GString *xml)
{
  GPtrArray *res = NULL;
  ParseContext *ctx = parse_context_new();
  GMarkupParseContext *pcontext = g_markup_parse_context_new(
      &parser_funcs, 0, ctx, (GDestroyNotify)parse_context_free);

  if (g_markup_parse_context_parse(pcontext, xml->str, xml->len, NULL)) {
    res = ctx->nodes;
    ctx->nodes = NULL;
  }

  g_markup_parse_context_free(pcontext);

  if (res) // NULL-terminate the array
    g_ptr_array_add(res, NULL);

  return res;
}
