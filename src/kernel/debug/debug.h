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
void SERIAL_Initialize(void);
void SERIAL_Putc(char c);
void SERIAL_Write(const char* s);

/* logging */
void log_printf(log_level_t level, const char* fmt, ...);
