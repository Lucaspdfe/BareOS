#include <stdint.h>
#include <hal/hal.h>
#include "stdio.h"


void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);
    
    clrscr();
    printf("Hello, world from KERNEL!!!!");

    for (;;);
}
