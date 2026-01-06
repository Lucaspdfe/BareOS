#include <stdint.h>
#include <stdio.h>
#include <hal/hal.h>
#include <hal/vfs.h>
#include <arch/i686/key.h>
#include <arch/i686/disp.h>
#include <arch/i686/fat.h>
#include <arch/i686/io.h>
#include <arch/i686/sched.h>

#define PROGRAM1_LOAD_ADDR  ((uint32_t)0x00200000)
#define PROGRAM2_LOAD_ADDR  ((uint32_t)0x00210000)

void load_and_add(const char* path, uint32_t load_addr) {
    int fd = i686_FAT_Open(path);
    if (fd < 0) {
        printf("Failed to open %s\n", path);
        return;
    }

    FAT_File* file = i686_FAT_GetFileInfo(fd);
    
    i686_FAT_Read(fd, (void*)load_addr, file->entry.Size);

    i686_FAT_Close(fd);

    i686_DisableInterrupts();
    /* Treat program entry as task entry */
    i686_SCHED_AddTask((Task)load_addr);
    i686_EnableInterrupts();
}

void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);

    printf("Hello, world!\n");

    // Add tasks here:
    load_and_add("/usr/prog.bin",  PROGRAM1_LOAD_ADDR);
    // load_and_add("/usr/prog2.bin", PROGRAM2_LOAD_ADDR);
}
