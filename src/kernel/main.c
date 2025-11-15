#include <stdint.h>
#include <hal/hal.h>
#include <arch/i686/debug.h>
#include <arch/i686/disk.h>
#include <arch/i686/fat.h>
#include "stdio.h"

void crash_me();

void readFile(const char* path) {
    int fd = i686_FAT_Open(path);
    FAT_File* file = i686_FAT_GetFileInfo(fd);

    uint8_t buffer[512];
    i686_FAT_Read(fd, &buffer, file->entry.Size);

    printf("Contents of %s: ", path);

    for (int i = 0; i < file->entry.Size; i++) {
        putc((char)buffer[i]);
    }

    putc('\n');
}

void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);

    readFile("/test.txt");
    readFile("/usr/test.txt");
    
    for (;;);
}
