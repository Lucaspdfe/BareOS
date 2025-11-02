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
    uint32_t pitch; // bytes per horizontal line (provided by bootloader VBE modeInfo->pitch)
    // Color masks/positions (from VBE mode info)
    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
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
