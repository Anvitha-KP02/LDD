#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
    debug_print(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) ((void)0)
#endif

void print_banner(void);
void print_error(const char *message);
void print_success(const char *message);
void print_info(const char *message);

bool read_int(const char *prompt, int *out_value);
bool read_int_in_range(const char *prompt, int min, int max, int *out_value);
void clear_stdin_buffer(void);

void log_message(const char *level, const char *message);
void log_operation(const char *data_structure, const char *operation, const char *status);
void debug_print(const char *file, int line, const char *fmt, ...);

#endif
