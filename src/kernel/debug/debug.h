#pragma once
#include <stdint.h>

/* log levels */
typedef enum {
    LOG_PANIC,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
} log_level_t;

/* serial */
void serial_init(void);
void serial_putc(char c);
void serial_write(const char* s);

/* logging */
void log_printf(log_level_t level, const char* fmt, ...);
