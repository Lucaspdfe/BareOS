#include <stdint.h>
#include <stdio.h>
#include <hal/hal.h>
#include <hal/vfs.h>
#include <arch/i686/key.h>
#include <arch/i686/disp.h>

#define PROGRAM_LOAD_ADDR  ((uint32_t)0x00200000)
#define PROGRAM_STACK_TOP  ((uint32_t)0x0020F000)

void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);

    printf("Hello, world!\n");
}
