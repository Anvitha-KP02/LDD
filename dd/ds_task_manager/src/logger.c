#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static FILE *g_log_file = NULL;

static const char *level_to_string(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_INFO:
            return "INFO";
        case LOG_LEVEL_WARN:
            return "WARN";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

int logger_init(const char *file_path) {
    g_log_file = fopen(file_path, "a");
    return g_log_file != NULL ? 0 : -1;
}

void logger_close(void) {
    if (g_log_file != NULL) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

void vlog_message(LogLevel level, const char *fmt, va_list args) {
    char ts[64];
    time_t now = time(NULL);
    struct tm tm_now;
    struct tm *tmp = localtime(&now);

    if (tmp == NULL) {
        strncpy(ts, "1970-01-01 00:00:00", sizeof(ts));
        ts[sizeof(ts) - 1] = '\0';
    } else {
        tm_now = *tmp;
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm_now);
    }

    if (g_log_file != NULL) {
        fprintf(g_log_file, "[%s] [%s] ", ts, level_to_string(level));
        vfprintf(g_log_file, fmt, args);
        fputc('\n', g_log_file);
        fflush(g_log_file);
    }
}

void log_message(LogLevel level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog_message(level, fmt, args);
    va_end(args);
}
