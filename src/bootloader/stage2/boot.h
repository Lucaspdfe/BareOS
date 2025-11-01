#pragma once

#include <stdint-gcc.h>

#define TAG_TYPE_FB    1
#define TAG_TYPE_DISK  2
#define TAG_TYPE_END   0xFF

typedef struct TAGHeader {
    uint8_t type;
    uint32_t size;
} TAGHeader;

typedef struct TAG_FB {
    TAGHeader header;
    uint8_t* fb;
    uint8_t bpp;
    uint32_t width, height;
} TAG_FB;

typedef struct TAG_DISK {
    TAGHeader header;
    uint8_t id;
    uint8_t type;
} TAG_DISK;

typedef struct TAG_Start {
    uint8_t totalTags;
    // Tags start here
} TAG_Start;
