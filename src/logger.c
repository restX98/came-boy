#include "logger.h"

#include <stdarg.h>
#include <stdio.h>

static const char *level_strings[] = { "DEBUG", "INFO", "WARN", "ERROR" };

static log_level_t min_level = LOG_DEBUG;

void log_msg(log_level_t level, const char *file, int line, const char *fmt, ...) {
    if (level < min_level) {
        return;
    }
    fprintf(stderr, "[%s]\t%s:%d: ", level_strings[level], file, line);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}
