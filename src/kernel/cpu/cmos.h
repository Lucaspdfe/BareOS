#pragma once
#include <stdint.h>

// Reads the current time and date from the CMOS RTC and stores it in the provided pointers. The year is returned as a full 4-digit year (e.g. 2024).
void CMOS_RTCRead(uint32_t* sec, uint32_t* min, uint32_t* hour, uint32_t* day, uint32_t* month, uint32_t* year);

// Returns the type of the floppy disks in drive 0 and drive 1. 0 = None, 1 = 360KB, 2 = 1.2MB, 3 = 720KB, 4 = 1.44MB, 5 = 2.88MB
void CMOS_GetFloppyType(uint8_t* drive0, uint8_t* drive1);
