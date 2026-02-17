#include <stdint.h>
#include <stddef.h>
#include "stdio.h"
#include "x86.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "vbe.h"

typedef void (*KernelStart)(uint8_t);

void __attribute__((cdecl)) start(uint16_t bootDrive)
{
    clrscr();

    DISK disk;
    if (!DISK_Initialize(&disk, bootDrive)) {
        printf("Disk init error\r\n");
        goto end;
    }

    if (!FAT_Initialize(&disk)) {
        printf("FAT init error\r\n");
        goto end;
    }

    // --- Load kernel ---
    FAT_File* fd = FAT_Open(&disk, "/boot/kernel.bin");
    if (!fd) {
        printf("boot/kernel.bin not found!!!");
        goto end;
    }
    uint8_t* kernelBuffer = (uint8_t*)MEMORY_KERNEL_ADDR;
    uint32_t read;
    while ((read = FAT_Read(&disk, fd, MEMORY_LOAD_SIZE, (void*)MEMORY_LOAD_KERNEL))) {
        memcpy(kernelBuffer, (void*)MEMORY_LOAD_KERNEL, read);
        kernelBuffer += read;
    }
    FAT_Close(fd);

    // --- Jump to kernel ---
    KernelStart kernelStart = (KernelStart)MEMORY_KERNEL_ADDR;
    kernelStart((uint8_t)bootDrive);

end:
    for (;;);
}
