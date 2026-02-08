#pragma once

#include <stdint.h>

#define TAG_TYPE_DISK  0x02
#define TAG_TYPE_END   0xFF

typedef struct TAGHeader {
    uint8_t type;
    uint32_t size;
} TAGHeader;

typedef struct TAG_DISK {
    TAGHeader header;
    uint8_t disks;
    uint8_t id[];
} TAG_DISK;

typedef struct TAG_Start {
    uint8_t totalTags;
    // Tags start here
} TAG_Start;
