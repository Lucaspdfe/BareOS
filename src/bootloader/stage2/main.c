#include <stdint.h>
#include <stddef.h>
#include "stdio.h"
#include "x86.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "vbe.h"
#include <boot/boot.h> // include your TAG_* definitions

typedef void (*KernelStart)(void*);

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

    // --- Build tags in memory (array, no fixed addr) ---
    uint8_t tagBuffer[256];
    memset(tagBuffer, 0, sizeof(tagBuffer));

    TAG_Start* tags = (TAG_Start*)tagBuffer;
    uint8_t* ptr = (uint8_t*)(tags + 1);
    tags->totalTags = 0;

    // --- Disk tag ---
    uint8_t disk_count = 1;
    size_t disk_tag_size = sizeof(TAG_DISK) + disk_count * sizeof(uint8_t);

    TAG_DISK* tag_disk = (TAG_DISK*)ptr;
    tag_disk->header.type = TAG_TYPE_DISK;
    tag_disk->header.size = disk_tag_size;
    tag_disk->disks = disk_count;
    tag_disk->id[0] = bootDrive;
    ptr += disk_tag_size;
    tags->totalTags++;

    // --- End tag ---
    TAGHeader* tag_end = (TAGHeader*)ptr;
    tag_end->type = TAG_TYPE_END;
    tag_end->size = sizeof(TAGHeader);
    ptr += sizeof(TAGHeader);
    tags->totalTags++;

    // --- Jump to kernel ---
    KernelStart kernelStart = (KernelStart)MEMORY_KERNEL_ADDR;
    kernelStart((void*)tags);

end:
    for (;;);
}
