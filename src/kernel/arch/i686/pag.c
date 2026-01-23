#include "pag.h"
#include <stdint.h>

typedef uint32_t pte_t;
typedef uint32_t pde_t;

#define PAGE_P   0x001
#define PAGE_RW  0x002
#define PAGE_US  0x004

__attribute__((aligned(4096)))
pde_t PageDirectory[1024];

__attribute__((aligned(4096)))
pte_t PageTable0[1024];   // kernel mapping

__attribute__((aligned(4096)))
pte_t PageTable1[1024];   // user mapping

__attribute__((aligned(4096)))
pte_t PageTable3[1024];   // heap mapping

__attribute__((aligned(4096)))
pte_t PageTable4[1024];   // heap mapping

__attribute__((aligned(4096)))
pte_t PageTableFB[1024];  // framebuffer mapping

void i686_PAG_Setup();

void i686_PAG_Initialize(TAG_FB* fb) {

    /* Clear page directory */
    for (int i = 0; i < 1024; i++)
        PageDirectory[i] = 0;

    /* Kernel identity-map first 4 MiB */
    for (int i = 0; i < 1024; i++)
        PageTable0[i] = (i * 0x1000) | (PAGE_P | PAGE_RW);

    PageDirectory[0] = ((uint32_t)PageTable0) | (PAGE_P | PAGE_RW);

    /* Clear user page table */
    for (int i = 0; i < 1024; i++)
        PageTable1[i] = 0;

    /* User mappings */
    PageTable1[(0x00500000 >> 12) & 0x3FF] = 0x00500000 | (PAGE_P | PAGE_RW | PAGE_US);
    PageTable1[(0x00510000 >> 12) & 0x3FF] = 0x00510000 | (PAGE_P | PAGE_RW | PAGE_US);

    PageDirectory[0x00500000 >> 22] =
        ((uint32_t)PageTable1) | (PAGE_P | PAGE_RW | PAGE_US);

    /* ================================
       Heap identity mapping (4 MiB)
       ================================ */

    /* Clear heap page table */
    for (int i = 0; i < 1024; i++) {
        PageTable3[i] = 0;
        PageTable4[i] = 0;
    }

    /* Identity-map entire PageTable2 */
    uint32_t heap_base = 0x00C00000;  /* must be 4 MiB aligned */
    uint32_t heap2_base = 0x01000000;  /* must be 4 MiB aligned */

    for (int i = 0; i < 1024; i++) {
        PageTable3[i] =
            (heap_base + (i * 0x1000)) | (PAGE_P | PAGE_RW);
        PageTable4[i] =
            (heap2_base + (i * 0x1000)) | (PAGE_P | PAGE_RW);
    }

    /* Hook heap page table into page directory */
    PageDirectory[heap_base >> 22] =
        ((uint32_t)PageTable3) | (PAGE_P | PAGE_RW);
    PageDirectory[heap2_base >> 22] =
        ((uint32_t)PageTable4) | (PAGE_P | PAGE_RW);

    /* ================================
       Framebuffer identity mapping
       ================================ */

    /* Clear framebuffer page table */
    for (int i = 0; i < 1024; i++)
        PageTableFB[i] = 0;

    uint32_t fb_phys  = (uint32_t)fb->fb;
    uint32_t fb_size  = fb->pitch * fb->height;

    uint32_t fb_start = fb_phys & 0xFFFFF000;
    uint32_t fb_end   = (fb_phys + fb_size + 0xFFF) & 0xFFFFF000;

    /* Map each 4 KiB page */
    for (uint32_t addr = fb_start; addr < fb_end; addr += 0x1000) {
        uint32_t pte = (addr >> 12) & 0x3FF;
        PageTableFB[pte] = addr | (PAGE_P | PAGE_RW);
    }

    /* Hook framebuffer page table */
    PageDirectory[fb_start >> 22] =
        ((uint32_t)PageTableFB) | (PAGE_P | PAGE_RW);

    /* Enable paging */
    i686_PAG_Setup();
}
