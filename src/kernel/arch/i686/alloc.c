#include "alloc.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    void* ptr;
    size_t size;
    bool isUsed;
} Page;

Page pages[256];

void i686_ALO_Initialize() {
    pages[0].isUsed = false;
    pages[0].ptr    = (void*)0x00500000;
    pages[0].size   = (size_t)0x000FFFFF;
}

void* i686_ALO_Malloc(size_t size) {
    for (int i = 0; i < 256; i++) {
        if (!pages[i].isUsed) {
            if (pages[i].size >= size) {

                pages[i].isUsed = true;
                size_t extraSize = pages[i].size - size;
                if (extraSize == 0) {
                    return pages[i].ptr;
                }

                // Split this page into: used part + free remainder
                pages[i].size = size;

                for (int j = 0; j < 256; j++) {
                    if (!pages[j].isUsed && pages[j].ptr == 0) {
                        pages[j].ptr    = (void*)((uintptr_t)pages[i].ptr + size);
                        pages[j].size   = extraSize;
                        pages[j].isUsed = false;
                        break;
                    }
                }

                return pages[i].ptr;
            }
        }
    }
    return (void*)0;
}

void i686_ALO_Free(void* ptr) {
    for (int i = 0; i < 256; i++) {
        if (pages[i].ptr == ptr) {
            pages[i].isUsed = false;

            // Merge with next page if adjacent and free
            for (int j = 0; j < 256; j++) {
                if (!pages[j].isUsed) {
                    if ((uintptr_t)pages[i].ptr + pages[i].size == (uintptr_t)pages[j].ptr) {
                        pages[i].size += pages[j].size;
                        pages[j].ptr   = 0;
                        pages[j].size  = 0;
                    }
                }
            }

            // Merge with previous page if adjacent and free
            for (int j = 0; j < 256; j++) {
                if (!pages[j].isUsed && pages[j].ptr != 0) {
                    if ((uintptr_t)pages[j].ptr + pages[j].size == (uintptr_t)pages[i].ptr) {
                        pages[j].size += pages[i].size;
                        pages[i].ptr   = 0;
                        pages[i].size  = 0;
                    }
                }
            }

            return;
        }
    }
}
