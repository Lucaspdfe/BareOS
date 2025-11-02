#pragma once
#include <stdint.h>
#include <util/boot.h>

void i686_DISP_Initialize(TAG_FB* fb);
void i686_DISP_PutPixel(uint32_t x, uint32_t y, uint32_t pixel);
void i686_DISP_PutChar(char c);
void i686_DISP_PutString(const char* s);
