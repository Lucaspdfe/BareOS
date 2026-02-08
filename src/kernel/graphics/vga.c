#include "vga.h"
#include <stdint.h>

volatile uint16_t* vga = (volatile uint16_t*)0x000B8000;

uint32_t x = 0;
uint32_t y = 0;
uint8_t color = 0x0F;

#define WIDTH  80
#define HEIGHT 25

void scroll(void) {
    for (uint32_t row = 1; row < HEIGHT; row++) {
        for (uint32_t col = 0; col < WIDTH; col++) {
            vga[(row - 1) * WIDTH + col] =
                vga[row * WIDTH + col];
        }
    }

    for (uint32_t col = 0; col < WIDTH; col++) {
        vga[(HEIGHT - 1) * WIDTH + col] =
            (uint16_t)' ' | ((uint16_t)color << 8);
    }
}

static inline void cursor_fix(void) {
    if (x >= WIDTH) {
        x = 0;
        y++;
    }

    if (y >= HEIGHT) {
        scroll();
        y = HEIGHT - 1;
    }
}

void putchar(uint32_t x, uint32_t y, char c) {
    vga[y * WIDTH + x] =
        (uint16_t)c | ((uint16_t)color << 8);
}

void putc(char c) {
    switch (c) {
        case '\n':
            x = 0;
            y++;
            break;

        case '\b':
            if (x > 0) {
                x--;
            } else if (y > 0) {
                y--;
                x = WIDTH - 1;
            }
            putchar(x, y, ' ');
            break;

        default:
            putchar(x, y, c);
            x++;
            break;
    }

    cursor_fix();
}

void puts(const char* s) {
    while (*s) {
        putc(*s++);
    }
}
