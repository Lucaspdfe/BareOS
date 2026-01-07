#pragma once
#include <stdint.h>

void i686_SPK_On(uint32_t freq);
void i686_SPK_Off(void);
void i686_SPK_Beep(uint32_t freq, uint32_t ms);
