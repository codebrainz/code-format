#include "style.h"

static struct
{
  const char *name;
  const char *label;
  const char *cmd_name;
} fmt_style_info[] = {
  { "custom", "Custom '.clang-format' File", "file" },
  { "llvm", "LLVM", "LLVM" }, { "google", "Google", "Google" },
  { "chromium", "Chromium", "Chromium" }, { "mozilla", "Mozilla", "Mozilla" },
  { "webkit", "WebKit", "WebKit" },
  // ...
};

size_t fmt_style_get_count(void)
{
  return G_N_ELEMENTS(fmt_style_info);
}

FmtStyle fmt_style_from_name(const char *name)
{
  for (size_t i = 0; i < G_N_ELEMENTS(fmt_style_info); i++) {
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
