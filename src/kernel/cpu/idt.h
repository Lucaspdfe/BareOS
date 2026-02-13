#pragma once
#include <stdint.h>

typedef enum IDT_ATTRIBUTES {
    IDT_ATTRIBUTE_PRESENT       = (1 << 7),

    IDT_ATTRIBUTE_DPL0          = (0b00 << 5),
    IDT_ATTRIBUTE_DPL1          = (0b01 << 5),
    IDT_ATTRIBUTE_DPL2          = (0b10 << 5),
    IDT_ATTRIBUTE_DPL3          = (0b11 << 5),

    IDT_ATTRIBUTE_TASK_GATE     = 0x05,
    IDT_ATTRIBUTE_16BIT_INT     = 0x06,
    IDT_ATTRIBUTE_16BIT_TRAP    = 0x07,
    IDT_ATTRIBUTE_32BIT_INT     = 0x0E,
    IDT_ATTRIBUTE_32BIT_TRAP    = 0x0F,
} IDT_ATTRIBUTES;

void IDT_Initialize();
void IDT_AddEntry(uint8_t interrupt, void* base, uint16_t codeSegment, uint8_t attributes);
void IDT_DisableGate(uint8_t interrupt);
void IDT_EnableGate(uint8_t interrupt);
