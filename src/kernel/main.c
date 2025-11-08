#include <stdint.h>
#include <hal/hal.h>
#include "stdio.h"
#include <arch/i686/irq.h>

void crash_me();

void timer(Registers* regs) {
    putc('.');
}

void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);
    
    clrscr();
    i686_IRQ_RegisterHandler(0, timer);
    clrscr();
    printf("Hello, world from KERNEL!!!!\n");

    // printf("Executing int 0x80 (Crashing OS): \n");
    // crash_me();

    for (;;);
}
