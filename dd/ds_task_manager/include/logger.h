#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} LogLevel;

int logger_init(const char *file_path);
void logger_close(void);
void log_message(LogLevel level, const char *fmt, ...);
void vlog_message(LogLevel level, const char *fmt, va_list args);

#endif
