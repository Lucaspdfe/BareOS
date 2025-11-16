// GDT.c
#include "gdt.h"
#include <stdint.h>
#include <string.h> /* memset */

/* ---- 32-bit TSS definition ---- */
typedef struct {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) TSS32;

/* Global TSS instance (aligned to 16 for cleanliness) */
static TSS32 g_TSS __attribute__((aligned(16)));

/* ---- existing GDT structures (kept as in your file) ---- */
typedef struct
{
    uint16_t LimitLow;                  // limit (bits 0-15)
    uint16_t BaseLow;                   // base (bits 0-15)
    uint8_t BaseMiddle;                 // base (bits 16-23)
    uint8_t Access;                     // access
    uint8_t FlagsLimitHi;               // limit (bits 16-19) | flags
    uint8_t BaseHigh;                   // base (bits 24-31)
} __attribute__((packed)) GDTEntry;

typedef struct
{
    uint16_t Limit;                     // sizeof(gdt) - 1
    GDTEntry* Ptr;                      // address of GDT
} __attribute__((packed)) GDTDescriptor;

/* Access and flag enums (unchanged) */
typedef enum
{
    GDT_ACCESS_CODE_READABLE                = 0x02,
    GDT_ACCESS_DATA_WRITEABLE               = 0x02,

    GDT_ACCESS_CODE_CONFORMING              = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL        = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN          = 0x04,

    GDT_ACCESS_DATA_SEGMENT                 = 0x10,
    GDT_ACCESS_CODE_SEGMENT                 = 0x18,

    GDT_ACCESS_DESCRIPTOR_TSS               = 0x00,

    GDT_ACCESS_RING0                        = 0x00,
    GDT_ACCESS_RING1                        = 0x20,
    GDT_ACCESS_RING2                        = 0x40,
    GDT_ACCESS_RING3                        = 0x60,

    GDT_ACCESS_PRESENT                      = 0x80,

} GDT_ACCESS;

typedef enum
{
    GDT_FLAG_64BIT                          = 0x20,
    GDT_FLAG_32BIT                          = 0x40,
    GDT_FLAG_16BIT                          = 0x00,

    GDT_FLAG_GRANULARITY_1B                 = 0x00,
    GDT_FLAG_GRANULARITY_4K                 = 0x80,
} GDT_FLAGS;

/* Helper macros */
#define GDT_LIMIT_LOW(limit)                (limit & 0xFFFF)
#define GDT_BASE_LOW(base)                  (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base)               ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)    (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base)                 ((base >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) {                     \
    GDT_LIMIT_LOW(limit),                                           \
    GDT_BASE_LOW(base),                                             \
    GDT_BASE_MIDDLE(base),                                          \
    access,                                                         \
    GDT_FLAGS_LIMIT_HI(limit, flags),                               \
    GDT_BASE_HIGH(base)                                             \
}

/* ---- GDT array: add TSS descriptor as last entry (index 5) ----
   Indices:
     0: null
     1: kernel code  (sel 0x08)
     2: kernel data  (sel 0x10)
     3: user code    (sel 0x18)
     4: user data    (sel 0x20)
     5: tss          (sel 0x28)
*/
// Placeholder TSS descriptor (base = 0 for now)
GDTEntry g_GDT[] = {
    GDT_ENTRY(0, 0, 0, 0), // null

    GDT_ENTRY(0, 0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 |
              GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    GDT_ENTRY(0, 0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 |
              GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    GDT_ENTRY(0, 0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 |
              GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    GDT_ENTRY(0, 0xFFFFF,
              GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 |
              GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITEABLE,
              GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K),

    // TSS descriptor (base/limit fixed at runtime)
    { 0, 0, 0, 0x89, 0, 0 },  // access = 0x89 (available 32-bit TSS)
};

GDTDescriptor g_GDTDescriptor = { sizeof(g_GDT) - 1, g_GDT };

/* Prototype for assembly function you already have that loads GDT and reloads segments */
void __attribute__((cdecl)) i686_GDT_Load(GDTDescriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment);

/* Initialize GDT and load TSS */
void i686_GDT_Initialize() {
    /* Compute address */
    uint32_t base = (uint32_t)&g_TSS;
    uint32_t limit = sizeof(TSS32) - 1;

    /* Patch GDT entry 5 (TSS) */
    g_GDT[5].BaseLow    =  base        & 0xFFFF;
    g_GDT[5].BaseMiddle = (base >> 16) & 0xFF;
    g_GDT[5].BaseHigh   = (base >> 24) & 0xFF;

    g_GDT[5].LimitLow   =  limit       & 0xFFFF;
    g_GDT[5].FlagsLimitHi =
        ((limit >> 16) & 0x0F) | (GDT_FLAG_GRANULARITY_1B); // 1-byte granularity


    memset(&g_TSS, 0, sizeof(g_TSS));
    g_TSS.ss0 = 0x10;         // kernel data selector
    g_TSS.esp0 = 0x00300000;  // your kernel stack top
    g_TSS.iomap_base = sizeof(TSS32);

    i686_GDT_Load(&g_GDTDescriptor, i686_GDT_CODE_SEGMENT, i686_GDT_DATA_SEGMENT);

    uint16_t tss_selector = 0x28; // index 5 * 8
    __asm__ volatile("ltr %0" : : "r"(tss_selector));
}
