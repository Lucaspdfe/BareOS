#include <stdint.h>
#include <stddef.h>
#include "memory.h"
#include <util/boot.h>
#include <arch/i686/disp.h>
#include "stdio.h"

extern uint8_t __bss_start;
extern uint8_t __bss_end;

void __attribute__((section(".entry"))) start(void* tags) {
    memset(&__bss_start, 0, (uintptr_t)&__bss_end - (uintptr_t)&__bss_start);

    TAG_Start* startTag = (TAG_Start*)tags;
    TAG_FB* fb = NULL;
    TAG_DISK* disk = NULL;
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
                    disk = (TAG_DISK*)header;
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

    i686_DISP_Initialize(fb);
    printf("Hello, world from KERNEL!!!!");

    for (;;);
}
