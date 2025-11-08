#include <stdint.h>
#include <hal/hal.h>
#include "stdio.h"

void crash_me();

void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);
    
    clrscr();
    printf("Hello, world from KERNEL!!!!\n");
    printf("Executing int 0x80 (Crashing OS): \n");

    crash_me();

    for (;;);
}
