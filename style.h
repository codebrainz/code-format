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
