#include "debug.h"
#include "io.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#define COM1 0x3F8

// ANSI color codes
#define COLOR_CRIT  "\033[31;1m"   // dark red (bright red without reset yet)
#define COLOR_ERR   "\033[31m"     // red
#define COLOR_WARN  "\033[33m"     // yellow
#define COLOR_INFO  "\033[0m"      // white / reset
#define COLOR_DEBUG "\033[90m"     // dark gray
#define COLOR_RESET "\033[0m"

static bool debug_initialized = false;

void i686_DEBUG_Initialize() {
    i686_outb(COM1 + 1, 0x00); // Disable all interrupts
    i686_outb(COM1 + 3, 0x80); // Enable DLAB
    i686_outb(COM1 + 0, 0x01); // Baud divisor low byte (115200)
    i686_outb(COM1 + 1, 0x00); // Baud divisor high byte
    i686_outb(COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
    i686_outb(COM1 + 2, 0xC7); // Enable FIFO, clear, 14-byte threshold
    i686_outb(COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
    debug_initialized = true;
}

void i686_DEBUG_Debugc(char c) {
    while ((i686_inb(COM1 + 5) & 0x20) == 0); // Wait for transmit ready
    i686_outb(COM1, c);
}

void i686_DEBUG_Debugs(const char* s) {
    while (*s) {
        i686_DEBUG_Debugc(*s++);
    }
}

void i686_DEBUG_Debugf(int level, const char* fmt, ...) {
    const char* color;
    const char* prefix;

    switch (level) {
        case LOG_CRIT:  color = COLOR_CRIT;  prefix = "[CRIT] ";  break;
        case LOG_ERR:   color = COLOR_ERR;   prefix = "[ERR] ";   break;
        case LOG_WARN:  color = COLOR_WARN;  prefix = "[WARN] ";  break;
        case LOG_INFO:  color = COLOR_INFO;  prefix = "[INFO] ";  break;
        case LOG_DEBUG: color = COLOR_DEBUG; prefix = "[DEBUG] "; break;
        default:        color = COLOR_RESET; prefix = "[LOG] ";   break;
    }

    i686_DEBUG_Debugs(color);
    i686_DEBUG_Debugs(prefix);

    char buffer[512];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len > 0) {
        i686_DEBUG_Debugs(buffer);
    }

    i686_DEBUG_Debugs(COLOR_RESET);
    i686_DEBUG_Debugc('\n');
}
