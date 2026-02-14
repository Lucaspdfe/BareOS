#include "pit.h"
#include "irq.h"
#include <io/io.h>

#define PIT_COMMAND  0x43
#define PIT_CHANNEL0 0x40

uint64_t ticks;

void PIT_Handler(Registers* regs);

void PIT_Initialize() {
    IRQ_RegisterHandler(0, PIT_Handler);
    uint16_t divisor = 1193182 / 1000;
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, divisor >> 8);
}

void PIT_Handler(Registers* regs) {
    ticks++;
}

void PIT_Sleep(uint32_t ms) {
    uint64_t start = ticks;
    uint64_t target = start + ms;
    while (ticks < target) {
        __asm__ volatile("hlt");
    }
}