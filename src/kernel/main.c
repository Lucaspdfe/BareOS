#include <stdint.h>
#include <hal/hal.h>
#include "stdio.h"
#include <arch/i686/pit.h>
#include <arch/i686/key.h>

void crash_me();

void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);

    clrscr();
    printf("Hello, world from KERNEL!!!!\n");

    for (;;);
}
