#include <graphics/printf.h>
#include <io/io.h>
#include <stdint.h>
#include <stdarg.h>

#define COM1 0x3F8

/* ANSI colors */
#define ANSI_RESET      "\x1b[0m"
#define ANSI_DARK_RED   "\x1b[31;1m"
#define ANSI_RED        "\x1b[31m"
#define ANSI_YELLOW     "\x1b[33m"
#define ANSI_WHITE      "\x1b[37m"
#define ANSI_LIGHT_GRAY "\x1b[37;2m"

/* log levels */
typedef enum {
    LOG_PANIC,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG
} log_level_t;

static const char* log_color[] = {
    [LOG_PANIC] = ANSI_DARK_RED,
    [LOG_ERROR] = ANSI_RED,
    [LOG_WARN]  = ANSI_YELLOW,
    [LOG_INFO]  = ANSI_WHITE,
    [LOG_DEBUG] = ANSI_LIGHT_GRAY
};

static const char* log_name[] = {
    "PANIC",
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG"
};

static inline int SERIAL_TXReady(void) {
    return inb(COM1 + 5) & 0x20;
}

void SERIAL_Initialize(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

void SERIAL_Putc(char c) {
    while (!SERIAL_TXReady())
        ;
    outb(COM1, (uint8_t)c);
}

void SERIAL_Write(const char* s) {
    while (*s) {
        if (*s == '\n')
            SERIAL_Putc('\r');
        SERIAL_Putc(*s++);
    }
}

void log_printf(log_level_t level, const char* fmt, ...) {
    char buf[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    SERIAL_Write(log_color[level]);
    SERIAL_Write("[");
    SERIAL_Write(log_name[level]);
    SERIAL_Write("] ");
    SERIAL_Write(buf);
    SERIAL_Write("\n");
    SERIAL_Write(ANSI_RESET);
}
