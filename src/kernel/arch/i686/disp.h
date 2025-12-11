#pragma once
#include <stdint.h>
#include <boot/boot.h>

void i686_DISP_Initialize(TAG_FB* fb);
void i686_DISP_Clear();
void i686_DISP_SetScale(uint32_t scale);
void i686_DISP_PutPixel(uint32_t x, uint32_t y, uint32_t pixel);
void i686_DISP_PutChar(char c);
void i686_DISP_PutString(const char* s);
