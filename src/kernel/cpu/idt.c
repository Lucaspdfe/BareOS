#include "idt.h"
#include <stdint.h>
#include <io/binary.h>

typedef struct IDTEntry {
    uint16_t BaseLow;
    uint16_t CodeSelector;
    uint8_t  _Reserved;
    uint8_t  Attributes;
    uint16_t BaseHigh;
} __attribute__((packed)) IDTEntry;

typedef struct IDTDescriptor {
    uint16_t Limit;
    IDTEntry* Ptr;
} __attribute__((packed)) IDTDescriptor;

IDTEntry g_IDT[256];
IDTDescriptor g_IDTDescriptor = { sizeof(g_IDT) - 1, g_IDT };

void __attribute__((cdecl)) IDT_Load(IDTDescriptor* Descriptor);

void IDT_Initialize() {
    IDT_Load(&g_IDTDescriptor);
}

void IDT_AddEntry(uint8_t interrupt, void* base, uint16_t codeSegment, uint8_t attributes) {
    IDTEntry* entry = &g_IDT[interrupt];
    uint32_t addr   = (uint32_t)base;

    entry->BaseLow = addr & 0xFFFF;
    entry->CodeSelector = codeSegment;
    entry->_Reserved = 0;
    entry->Attributes = attributes;
    entry->BaseHigh = (addr >> 16) & 0xFFFF;
}

void IDT_EnableGate(uint8_t interrupt) {
    g_IDT[interrupt].Attributes = FLAG_SET(g_IDT[interrupt].Attributes, IDT_ATTRIBUTE_PRESENT);
}

void IDT_DisableGate(uint8_t interrupt) {
    g_IDT[interrupt].Attributes = FLAG_SET(g_IDT[interrupt].Attributes, IDT_ATTRIBUTE_PRESENT);
}
