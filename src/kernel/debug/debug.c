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

static inline int serial_tx_ready(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

void serial_putc(char c) {
    while (!serial_tx_ready())
        ;
    outb(COM1, (uint8_t)c);
}

void serial_write(const char* s) {
    while (*s) {
        if (*s == '\n')
            serial_putc('\r');
        serial_putc(*s++);
    }
}

void log_printf(log_level_t level, const char* fmt, ...) {
    char buf[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    serial_write(log_color[level]);
    serial_write("[");
    serial_write(log_name[level]);
    serial_write("] ");
    serial_write(buf);
    serial_write("\n");
    serial_write(ANSI_RESET);
}
