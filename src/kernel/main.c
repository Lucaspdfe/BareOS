#include <stdint.h>
#include <hal/hal.h>
#include "stdio.h"
#include <arch/i686/fdc.h>
#include <hal/vfs.h>

void crash_me();

void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);

    uint8_t buffer[512];

    i686_FDC_ReadSectors(0, 0, 1, 1, &buffer);

    if (buffer[510] == 0x55 && buffer[511] == 0xAA) {
        printf("Works!!!");
    }

    for (;;);
}
