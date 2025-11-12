#pragma once
#include <stdint.h>

#define FDC_DMA_BUFFER_ADDR ((void*)0x30000)
#define FDC_SECTOR_SIZE     512

// Public API
void i686_FDC_Initialize();
void i686_FDC_ReadSector(uint8_t track, uint8_t head, uint8_t sector, void* buffer);
void i686_FDC_ReadSectors(uint8_t track, uint8_t head, uint8_t startSector, uint8_t count, void* buffer);
void i686_FDC_WriteSector(uint8_t track, uint8_t head, uint8_t sector, const void* buffer);
void i686_FDC_WriteSectors(uint8_t track, uint8_t head, uint8_t startSector, uint8_t count, const void* buffer);
