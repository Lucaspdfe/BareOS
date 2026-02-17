#include "disk.h"
#include "fdc.h"

void DISK_Initialize(uint8_t drive, DISK* disk) {
    disk->driveNumber = drive;
    disk->seekedLBA = 0;

    if (drive < 2) {
        disk->DriverToBeUsed = 0; // FDC
        FDC_Initialize(drive);
        FDC_GetInfo(drive, &disk->maxCylinders, &disk->maxHeads, &disk->maxSectors);
    } else {
        disk->maxCylinders = 0;
        disk->maxHeads = 0;
        disk->maxSectors = 0;
        disk->DriverToBeUsed = 0xFF; // No driver
    }
}

void DISK_ReadSectors(DISK* disk, uint8_t count, void* buffer) {
    if (disk->DriverToBeUsed == 0xFF) return; // No driver
    if (disk->DriverToBeUsed == 0) { // FDC
        uint8_t lba = disk->seekedLBA;
        uint8_t head = (lba / disk->maxSectors) % disk->maxHeads;
        uint8_t track = (lba / disk->maxSectors) / disk->maxHeads;
        uint8_t sector = (lba % disk->maxSectors) + 1; // Sectors are 1-based
        FDC_ReadSectors(disk->driveNumber, track, head, sector, count, buffer);
        disk->seekedLBA += count;
    }
}

void DISK_WriteSectors(DISK* disk, uint8_t count, const void* buffer) {
    if (disk->DriverToBeUsed == 0xFF) return; // No driver
    if (disk->DriverToBeUsed == 0) { // FDC
        uint8_t lba = disk->seekedLBA;
        uint8_t head = (lba / disk->maxSectors) % disk->maxHeads;
        uint8_t track = (lba / disk->maxSectors) / disk->maxHeads;
        uint8_t sector = (lba % disk->maxSectors) + 1; // Sectors are 1-based
        FDC_WriteSectors(disk->driveNumber, track, head, sector, count, buffer);
        disk->seekedLBA += count;
    }
}

void DISK_Seek(DISK* disk, uint8_t lba) {
    disk->seekedLBA = lba;
}
