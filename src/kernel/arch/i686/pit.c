#include "pit.h"
#include "irq.h"
#include "disp.h"
#include "sched.h"
#include "io.h"

#define PIT_COMMAND  0x43
#define PIT_CHANNEL0 0x40

uint64_t ticks;

void i686_PIT_Handler(Registers* regs);

void i686_PIT_Initialize() {
    i686_IRQ_RegisterHandler(0, i686_PIT_Handler);
    uint16_t divisor = 1193182 / 1000;
    i686_outb(PIT_COMMAND, 0x36);
    i686_outb(PIT_CHANNEL0, divisor & 0xFF);
    i686_outb(PIT_CHANNEL0, divisor >> 8);
}

void i686_PIT_Handler(Registers* regs) {
    ticks++;
    i686_DISP_ToggleCursor();

    if (preempt_disable == 0) {
        i686_SCHED_Schedule(regs);
    }
}

void i686_PIT_Sleep(uint32_t ms) {
    uint64_t start = ticks;
    uint64_t target = start + ms;
    while (ticks < target) {
        __asm__ volatile("hlt");
    }
}
