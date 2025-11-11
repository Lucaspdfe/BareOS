#include <stdint.h>
#include <hal/hal.h>
#include "stdio.h"
#include <arch/i686/pit.h>
#include <arch/i686/key.h>
#include <hal/vfs.h>

void crash_me();

void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);

    clrscr();
    char buffer[256];
    size_t amount = VFS_Read(STDIN, buffer, 256);
    printf("Buffer content: \"%s\" size: %i", buffer, amount);

    for (;;);
}
