#pragma once
#include <stdint.h>

void CMOS_RTCRead(uint32_t* sec, uint32_t* min, uint32_t* hour, uint32_t* day, uint32_t* month, uint32_t* year);
