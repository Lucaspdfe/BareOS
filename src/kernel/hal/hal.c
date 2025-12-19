#include "hal.h"
#include <string.h>
#include <arch/i686/debug.h>
#include <arch/i686/disp.h>
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>
#include <arch/i686/pit.h>
#include <arch/i686/key.h>
#include <arch/i686/disk.h>
#include <arch/i686/fat.h>
#include <arch/i686/sys.h>
#include <stddef.h>
#include <stdio.h>

extern uint8_t __bss_start;
extern uint8_t __bss_end;

void HAL_Initialize(void* tags) {
    memset(&__bss_start, 0, (uintptr_t)&__bss_end - (uintptr_t)&__bss_start);

    TAG_Start* startTag = (TAG_Start*)tags;
    TAG_FB* fb = NULL;
    TAG_DISK* disk_tag = NULL;
    if (startTag) {
        uint8_t count = startTag->totalTags;
        uint8_t* ptr = (uint8_t*)(startTag + 1);

        for (uint8_t i = 0; i < count; ++i) {
            TAGHeader* header = (TAGHeader*)ptr;
            if (!header) break;

            switch (header->type) {
                case TAG_TYPE_FB:
                    fb = (TAG_FB*)header;
                    break;
                case TAG_TYPE_DISK:
                    disk_tag = (TAG_DISK*)header;
                    break;
                case TAG_TYPE_END:
                    i = count; // stop early
                    break;
                default:
                    break;
            }

            ptr += header->size;
        }
    }

    i686_DEBUG_Initialize();
    i686_DEBUG_Debugf(LOG_DEBUG, "DEBUG Initialized.");
    i686_DISP_Initialize(fb);
    i686_DISP_SetScale(1);
    i686_DEBUG_Debugf(LOG_DEBUG, "DISP Initialized.");
    i686_GDT_Initialize();
    i686_DEBUG_Debugf(LOG_DEBUG, "GDT Initialized.");
    i686_IDT_Initialize();
    i686_DEBUG_Debugf(LOG_DEBUG, "IDT Initialized.");
    i686_ISR_Initialize();
    i686_DEBUG_Debugf(LOG_DEBUG, "ISR Initialized.");
    i686_IRQ_Initialize();
    i686_DEBUG_Debugf(LOG_DEBUG, "PIC Initialized.");       // PIC Initializes in conjunction with IRQ
    i686_DEBUG_Debugf(LOG_DEBUG, "IRQ Initialized.");
    i686_PIT_Initialize();
    i686_DEBUG_Debugf(LOG_DEBUG, "PIT Initialized.");
    i686_KEY_Initialize();
    i686_DEBUG_Debugf(LOG_DEBUG, "Keyboard Initialized.");
    DISK disk = i686_DISK_Initialize(disk_tag);
    i686_DEBUG_Debugf(LOG_DEBUG, "Disk Initialized.");
    if (!i686_FAT_Initialize(&disk)) {
        i686_DEBUG_Debugf(LOG_ERR, "FAT NOT Initialized.");
    }
    i686_DEBUG_Debugf(LOG_DEBUG, "FAT Initialized.");
    i686_SYS_Initialize();
    i686_DEBUG_Debugf(LOG_DEBUG, "SYSCALL Initialized.");
}
