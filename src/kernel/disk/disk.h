#pragma once
#include <stdint.h>

typedef struct DISK {
    uint8_t driveNumber;
    uint8_t DriverToBeUsed; // 0=FDC, 1=HDD, etc (for future expansion)
    uint8_t maxCylinders;
    uint8_t maxHeads;
    uint8_t maxSectors;
    uint8_t seekedLBA;
} DISK;

void DISK_Initialize(uint8_t drive, DISK* disk);
void DISK_ReadSectors(DISK* disk, uint8_t count, void* buffer);
void DISK_WriteSectors(DISK* disk, uint8_t count, const void* buffer);
void DISK_Seek(DISK* disk, uint8_t lba);
