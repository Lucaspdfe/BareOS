#pragma once
#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );
    return value;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

static inline void Panic() {
    __asm__ volatile (
        "cli; hlt"
    );
}

static inline void IOWait() {
    outb(0x80, 0);
}

static inline void EnableInterrupts() {
    __asm__ volatile (
        "sti"
    );
}

static inline void DisableInterrupts() {
    __asm__ volatile (
        "cli"
    );
}
