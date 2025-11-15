#include "disk.h"
#include "fdc.h"

DISK i686_DISK_Initialize(TAG_DISK* disk) {
    i686_FDC_Initialize();

    DISK disk_out;
    disk_out.sectors    = 18;
    disk_out.heads      = 2;
    disk_out.cylinders  = 80;
    disk_out.id         = disk->id;
    disk_out.type       = disk->type;
    return disk_out;
}

void DISK_LBA2CHS(DISK* disk, uint32_t lba,
                  uint16_t* cylinderOut, uint16_t* sectorOut, uint16_t* headOut)
{
    uint16_t S = 18;
    uint16_t H = 2;

    *cylinderOut = lba / (S * H);
    *headOut     = (lba / S) % H;
    *sectorOut   = (lba % S) + 1;
}

bool i686_DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, const void* dataOut)
{
    uint16_t cylinder, head, sector;
    uint8_t* buffer = (uint8_t*)dataOut;
    uint8_t remaining = sectors;
    uint32_t offset = 0;

    while (remaining > 0)
    {
        DISK_LBA2CHS(disk, lba + offset, &cylinder, &sector, &head);

        // Determine how many sectors we can read sequentially on the same track
        uint8_t maxSectors = disk->sectors - (sector - 1); // sectors left on this track
        uint8_t toRead = (remaining < maxSectors) ? remaining : maxSectors;

        // debugf(DBG_DEBUG, "C=%u H=%u S=%u, Reading %u sectors\n", cylinder, head, sector, toRead); <DEB>

        // Call new multi-sector FDC read
        i686_FDC_ReadSectors((uint8_t)cylinder, (uint8_t)head, (uint8_t)sector, toRead,
                              buffer + offset * 512);

        offset += toRead;
        remaining -= toRead;
    }

    return true;
}

bool i686_DISK_WriteSectors(DISK* disk, uint32_t lba, uint8_t sectors, const void* dataIn)
{
    uint16_t cylinder, head, sector;
    const uint8_t* buffer = (const uint8_t*)dataIn;
    uint8_t remaining = sectors;
    uint32_t offset = 0;

    while (remaining > 0)
    {
        // Convert LBA to CHS
        DISK_LBA2CHS(disk, lba + offset, &cylinder, &sector, &head);

        // Determine how many sectors we can write sequentially on the same track
        uint8_t maxSectors = disk->sectors - (sector - 1); // sectors left on this track
        uint8_t toWrite = (remaining < maxSectors) ? remaining : maxSectors;

        // Call multi-sector FDC write
        i686_FDC_WriteSectors((uint8_t)cylinder, (uint8_t)head, (uint8_t)sector, toWrite,
                               buffer + offset * 512);

        offset += toWrite;
        remaining -= toWrite;
    }

    return true;
}
