#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_CYAN "\033[36m"
#define COLOR_YELLOW "\033[33m"

static const char *LOG_FILE = "app.log";

void print_banner(void) {
    printf(COLOR_CYAN "============================================\n" COLOR_RESET);
    printf(COLOR_CYAN " Data Structures CLI (Stack/Queue/List)\n" COLOR_RESET);
    printf(COLOR_CYAN "============================================\n" COLOR_RESET);
}

void print_error(const char *message) {
    printf(COLOR_RED "[ERROR] %s\n" COLOR_RESET, message);
}

void print_success(const char *message) {
    printf(COLOR_GREEN "[OK] %s\n" COLOR_RESET, message);
}

void print_info(const char *message) {
    printf(COLOR_YELLOW "[INFO] %s\n" COLOR_RESET, message);
}

bool read_int(const char *prompt, int *out_value) {
    char buffer[128];
    char *end_ptr;
    long value;

    if (prompt == NULL || out_value == NULL) {
        return false;
    }

    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return false;
    }

    value = strtol(buffer, &end_ptr, 10);
    if (end_ptr == buffer) {
        return false;
    }

    while (*end_ptr == ' ' || *end_ptr == '\t') {
        end_ptr++;
    }
    if (*end_ptr != '\n' && *end_ptr != '\0') {
        return false;
    }

    if (value < -2147483648L || value > 2147483647L) {
        return false;
    }

    *out_value = (int)value;
    return true;
}

bool read_int_in_range(const char *prompt, int min, int max, int *out_value) {
    int value;
    if (!read_int(prompt, &value)) {
        return false;
    }
    if (value < min || value > max) {
        return false;
    }
    *out_value = value;
    return true;
}

void clear_stdin_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

void log_message(const char *level, const char *message) {
    FILE *file;
    time_t now;
    struct tm *tm_info;
    char timestamp[32];

    if (level == NULL || message == NULL) {
        return;
    }

    file = fopen(LOG_FILE, "a");
    if (file == NULL) {
        return;
    }

    now = time(NULL);
    tm_info = localtime(&now);
    if (tm_info == NULL) {
        fclose(file);
        return;
    }

    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(file, "[%s] [%s] %s\n", timestamp, level, message);
    fclose(file);
}

void log_operation(const char *data_structure, const char *operation, const char *status) {
    char buffer[256];

    if (data_structure == NULL || operation == NULL || status == NULL) {
        return;
    }

    snprintf(buffer, sizeof(buffer), "%s | %s | %s", data_structure, operation, status);
    log_message("INFO", buffer);
}

void debug_print(const char *file, int line, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "[DEBUG] %s:%d: ", file, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}
