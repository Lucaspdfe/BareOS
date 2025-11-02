#include <stdint.h>
#include "stdio.h"
#include "x86.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "vbe.h"
#include "boot.h" // include your TAG_* definitions

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

    // --- Set graphics mode ---
    const int desiredWidth = 1024, desiredHeight = 768, desiredBpp = 32;
    uint16_t pickedMode = 0xFFFF;

    VbeInfoBlock* info = (VbeInfoBlock*)MEMORY_VESA_INFO;
    VbeModeInfo* modeInfo = (VbeModeInfo*)MEMORY_MODE_INFO;

    if (VBE_GetControllerInfo(info)) {
        uint16_t* mode = (uint16_t*)(info->VideoModePtr);
        for (int i = 0; mode[i] != 0xFFFF; i++) {
            if (!VBE_GetModeInfo(mode[i], modeInfo))
                continue;

            bool hasFB = (modeInfo->attributes & 0x90) == 0x90;
            if (hasFB &&
                modeInfo->width == desiredWidth &&
                modeInfo->height == desiredHeight &&
                modeInfo->bpp == desiredBpp) {
                pickedMode = mode[i];
                break;
            }
        }

        if (pickedMode != 0xFFFF)
            VBE_SetMode(pickedMode);

        // No stage2 framebuffer drawing here; stage2 only sets up tags (including pitch and
        // color mask/position) which the kernel will use to draw.
    } else {
        printf("No VBE extensions :(\n");
        goto end;
    }

    // --- Build tags in memory (array, no fixed addr) ---
    uint8_t tagBuffer[256];
    memset(tagBuffer, 0, sizeof(tagBuffer));

    TAG_Start* tags = (TAG_Start*)tagBuffer;
    uint8_t* ptr = (uint8_t*)(tags + 1);
    tags->totalTags = 0;

    // --- Framebuffer tag ---
    TAG_FB* tag_fb = (TAG_FB*)ptr;
    tag_fb->header.type = TAG_TYPE_FB;
    tag_fb->header.size = sizeof(TAG_FB);
    tag_fb->fb = (uint8_t*)modeInfo->framebuffer;
    tag_fb->bpp = modeInfo->bpp;
    tag_fb->width = modeInfo->width;
    tag_fb->pitch = modeInfo->pitch;
    tag_fb->height = modeInfo->height;
    tag_fb->red_mask = modeInfo->red_mask;
    tag_fb->red_position = modeInfo->red_position;
    tag_fb->green_mask = modeInfo->green_mask;
    tag_fb->green_position = modeInfo->green_position;
    tag_fb->blue_mask = modeInfo->blue_mask;
    tag_fb->blue_position = modeInfo->blue_position;
    ptr += sizeof(TAG_FB);
    tags->totalTags++;

    // --- Disk tag ---
    TAG_DISK* tag_disk = (TAG_DISK*)ptr;
    tag_disk->header.type = TAG_TYPE_DISK;
    tag_disk->header.size = sizeof(TAG_DISK);
    tag_disk->id = bootDrive;
    tag_disk->type = 0; // 0 = floppy, etc.
    ptr += sizeof(TAG_DISK);
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
