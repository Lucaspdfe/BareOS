#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <util/boot.h>

typedef struct {
    uint8_t id;
    uint8_t type;
    uint8_t cylinders;
    uint8_t heads;
    uint8_t sectors;
} DISK;

DISK i686_DISK_Initialize(TAG_DISK* disk);
bool i686_DISK_WriteSectors(DISK* disk, uint32_t lba, uint8_t sectors, const void* dataIn);
#define DISK_WriteSectors i686_DISK_WriteSectors
bool i686_DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, const void* dataOut);
#define DISK_ReadSectors i686_DISK_ReadSectors
