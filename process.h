#ifndef FMT_PROCESS_H
#define FMT_PROCESS_H

#include "plugin.h"

G_BEGIN_DECLS

typedef struct FmtProcess FmtProcess;

FmtProcess *fmt_process_open(const char *work_dir, const char *const *argv);
int fmt_process_close(FmtProcess *proc);
bool fmt_process_run(FmtProcess *proc, const char *str_in, size_t in_len,
                     GString *str_out);

G_END_DECLS

#endif // FMT_PROCESS_H
