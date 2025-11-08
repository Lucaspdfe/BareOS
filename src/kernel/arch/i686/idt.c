#include "idt.h"
#include <stdint.h>
#include <util/binary.h>

typedef struct
{
    uint16_t BaseLow;
    uint16_t SegmentSelector;
    uint8_t Reserved;
    uint8_t Flags;
    uint16_t BaseHigh;
} __attribute__((packed)) IDTEntry;

typedef struct
{
    uint16_t Limit;
    IDTEntry* Ptr;
} __attribute__((packed)) IDTDescriptor;

IDTEntry g_IDTEntries[255];

IDTDescriptor g_IDTDescriptor = { sizeof(g_IDTEntries) - 1, g_IDTEntries };

void __attribute__((cdecl)) i686_IDT_Load(IDTDescriptor* idtDescriptor);

void i686_IDT_Initialize() {
    i686_IDT_Load(&g_IDTDescriptor);
}

void i686_IDT_DisableGate(int interrupt) {
    FLAG_UNSET(g_IDTEntries[interrupt].Flags, IDT_FLAG_PRESENT);
}

void i686_IDT_EnableGate(int interrupt) {
    FLAG_SET(g_IDTEntries[interrupt].Flags, IDT_FLAG_PRESENT);
}

void i686_IDT_SetGate(int interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags) {
    g_IDTEntries[interrupt].BaseLow = ((uint32_t)base) & 0xFFFF;
    g_IDTEntries[interrupt].SegmentSelector = segmentDescriptor;
    g_IDTEntries[interrupt].Reserved = 0;
    g_IDTEntries[interrupt].Flags = flags;
    g_IDTEntries[interrupt].BaseHigh = ((uint32_t)base >> 16) & 0xFFFF;
}
