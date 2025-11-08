#pragma once
#include <stdint.h>
#include <stdbool.h>

//  --- All of these functions are defined in asm, ignore the undefined note. ---
void __attribute__((cdecl)) i686_outb(uint16_t port, uint8_t value);
uint8_t __attribute__((cdecl)) i686_inb(uint16_t port);
void __attribute__((cdecl)) i686_Panic();
