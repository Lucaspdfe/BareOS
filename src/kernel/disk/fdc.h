#pragma once
#include <stdint.h>

void FDC_Initialize(uint8_t drive);
void FDC_ReadSector(uint8_t drive, uint8_t track, uint8_t head, uint8_t sector, void* buffer);
void FDC_ReadSectors(uint8_t drive, uint8_t track, uint8_t head, uint8_t startSector, uint8_t count, void* buffer);
void FDC_WriteSector(uint8_t drive, uint8_t track, uint8_t head, uint8_t sector, const void* buffer);
void FDC_WriteSectors(uint8_t drive, uint8_t track, uint8_t head, uint8_t startSector, uint8_t count, const void* buffer);
void FDC_GetInfo(uint8_t drive, uint8_t* maxCylinders, uint8_t* maxHeads, uint8_t* maxSectors);
