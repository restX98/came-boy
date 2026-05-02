#ifndef LOG_HELPERS_H
#define LOG_HELPERS_H

#include <stdio.h>

static FILE *original_stderr;

static void suppress_logs(void) {
    original_stderr = stderr;
    stderr = fopen("/dev/null", "w");
}

static void restore_logs(void) {
    if (original_stderr) {
        fclose(stderr);
        stderr = original_stderr;
        original_stderr = NULL;
    }
}

#endif // LOG_HELPERS_H
