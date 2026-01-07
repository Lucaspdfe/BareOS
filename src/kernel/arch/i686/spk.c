#include "spk.h"
#include "io.h"
#include "pit.h"

#define PIT_COMMAND  0x43
#define PIT_CH2      0x42
#define SPK_PORT     0x61

void i686_SPK_On(uint32_t freq) {
    if (freq == 0)
        return;

    uint16_t divisor = 1193182 / freq;

    // Configure PIT channel 2: square wave
    i686_outb(PIT_COMMAND, 0xB6);

    // Send divisor (low byte, then high byte)
    i686_outb(PIT_CH2, divisor & 0xFF);
    i686_outb(PIT_CH2, divisor >> 8);

    // Enable speaker (set bits 0 and 1)
    uint8_t val = i686_inb(SPK_PORT);
    if ((val & 3) != 3)
        i686_outb(SPK_PORT, val | 3);
}

void i686_SPK_Off(void) {
    uint8_t val = i686_inb(SPK_PORT);
    i686_outb(SPK_PORT, val & ~3);
}

void i686_SPK_Beep(uint32_t freq, uint32_t ms) {
    i686_SPK_On(freq);
    i686_PIT_Sleep(ms);
    i686_SPK_Off();
}
