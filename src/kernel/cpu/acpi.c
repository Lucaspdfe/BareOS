#include "acpi.h"
#include <io/io.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <debug/debug.h>

#define SLP_EN (1 << 13)
#define SCI_EN 1
#define ACPI_ENABLE_TIMEOUT 1000000

/* ============================================================
   STRUCTURES
   ============================================================ */

typedef struct {
    char     signature[8];
    uint8_t  checksum;
    char     oemid[6];
    uint8_t  revision;
    uint32_t rsdt_address;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t  extended_checksum;
    uint8_t  reserved[3];
} __attribute__((packed)) RSDPDescriptor;

typedef struct {
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     oemid[6];
    char     oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) SDTHeader;

/* Proper ACPI 1.0 FADT */
typedef struct {
    SDTHeader header;

    uint32_t firmware_ctrl;
    uint32_t dsdt;

    uint8_t  reserved;

    uint8_t  preferred_pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd_port;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint8_t  s4bios_req;
    uint8_t  pstate_cnt;

    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;
    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;
    uint32_t pm2_cnt_blk;
    uint32_t pm_tmr_blk;
    uint32_t gpe0_blk;
    uint32_t gpe1_blk;

    uint8_t  pm1_evt_len;
    uint8_t  pm1_cnt_len;
    uint8_t  pm2_cnt_len;
    uint8_t  pm_tmr_len;
    uint8_t  gpe0_blk_len;
    uint8_t  gpe1_blk_len;
    uint8_t  gpe1_base;
    uint8_t  cst_cnt;

    uint16_t p_lvl2_lat;
    uint16_t p_lvl3_lat;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t  duty_offset;
    uint8_t  duty_width;
    uint8_t  day_alrm;
    uint8_t  mon_alrm;
    uint8_t  century;

    uint16_t iapc_boot_arch;
    uint8_t  reserved2;
    uint32_t flags;

} __attribute__((packed)) FADT;

/* ============================================================ */

static RSDPDescriptor* rsdp = NULL;
static FADT* fadt = NULL;

/* ============================================================ */

static int acpi_checksum(void* addr, size_t len) {
    uint8_t sum = 0;
    uint8_t* p = addr;

    for (size_t i = 0; i < len; i++)
        sum += p[i];

    return sum == 0;
}

/* ============================================================
   RSDP SEARCH
   ============================================================ */

static void ACPI_SearchForRSDP() {

    uint16_t ebdaSeg = *(uint16_t*)0x40E;
    uint32_t ebda = ((uint32_t)ebdaSeg) << 4;

    for (uint32_t addr = ebda; addr < ebda + 1024; addr += 16) {
        RSDPDescriptor* desc = (RSDPDescriptor*)addr;

        if (!memcmp(desc->signature, "RSD PTR ", 8) &&
            acpi_checksum(desc, 20)) {

            rsdp = desc;
            return;
        }
    }

    for (uint32_t addr = 0xE0000; addr < 0x100000; addr += 16) {
        RSDPDescriptor* desc = (RSDPDescriptor*)addr;

        if (!memcmp(desc->signature, "RSD PTR ", 8) &&
            acpi_checksum(desc, 20)) {

            rsdp = desc;
            return;
        }
    }
}

/* ============================================================
   FIND FADT
   ============================================================ */

static void ACPI_FindFADT() {

    if (!rsdp)
        return;

    SDTHeader* sdt = NULL;
    uint32_t entries = 0;

    if (rsdp->revision >= 2 && rsdp->xsdt_address) {

        sdt = (SDTHeader*)(uintptr_t)rsdp->xsdt_address;
        entries = (sdt->length - sizeof(SDTHeader)) / 8;

        uint64_t* tablePtrs =
            (uint64_t*)((uint8_t*)sdt + sizeof(SDTHeader));

        for (uint32_t i = 0; i < entries; i++) {
            SDTHeader* hdr =
                (SDTHeader*)(uintptr_t)tablePtrs[i];

            if (!memcmp(hdr->signature, "FACP", 4)) {
                fadt = (FADT*)hdr;
                break;
            }
        }

    } else {

        sdt = (SDTHeader*)(uintptr_t)rsdp->rsdt_address;
        entries = (sdt->length - sizeof(SDTHeader)) / 4;

        uint32_t* tablePtrs =
            (uint32_t*)((uint8_t*)sdt + sizeof(SDTHeader));

        for (uint32_t i = 0; i < entries; i++) {
            SDTHeader* hdr =
                (SDTHeader*)(uintptr_t)tablePtrs[i];

            if (!memcmp(hdr->signature, "FACP", 4)) {
                fadt = (FADT*)hdr;
                break;
            }
        }
    }
}

/* ============================================================
   ENABLE ACPI
   ============================================================ */

static void ACPI_Enable() {

    if (!fadt || !fadt->smi_cmd_port)
        return;

    uint16_t pm1 = inw(fadt->pm1a_cnt_blk);

    if (!(pm1 & SCI_EN)) {

        outb(fadt->smi_cmd_port, fadt->acpi_enable);

        int timeout = ACPI_ENABLE_TIMEOUT;
        while (!(inw(fadt->pm1a_cnt_blk) & SCI_EN) && timeout--);
    }
}

/* ============================================================
   EXTRACT _S5
   ============================================================ */

static int ACPI_GetS5(uint16_t* slp_typ) {

    SDTHeader* dsdt =
        (SDTHeader*)(uintptr_t)fadt->dsdt;

    uint8_t* data =
        (uint8_t*)dsdt + sizeof(SDTHeader);

    uint32_t len =
        dsdt->length - sizeof(SDTHeader);

    for (uint32_t i = 0; i < len - 4; i++) {

        if (!memcmp(&data[i], "_S5_", 4)) {

            uint8_t* p = &data[i + 4];

            while (p < data + len && *p != 0x12)
                p++;

            if (p >= data + len)
                return -1;

            p++;        // PackageOp
            p++;        // PkgLength
            p++;        // NumElements

            if (*p == 0x0A)
                p++;

            *slp_typ = *p;
            return 0;
        }
    }

    return -1;
}

/* ============================================================
   INITIALIZE
   ============================================================ */

void ACPI_Initialize() {

    ACPI_SearchForRSDP();
    if (!rsdp) {
        log_printf(LOG_WARN, "ACPI RSDP not found");
        return;
    }
    ACPI_FindFADT();
}

/* ============================================================
   POWER OFF
   ============================================================ */

void ACPI_PowerOff() {

    if (!fadt)
        return;

    uint16_t slp_typ;

    if (ACPI_GetS5(&slp_typ) != 0)
        return;

    ACPI_Enable();

    uint16_t value = (slp_typ << 10) | SLP_EN;

    outw(fadt->pm1a_cnt_blk, value);

    if (fadt->pm1b_cnt_blk)
        outw(fadt->pm1b_cnt_blk, value);

    for (;;)
        __asm__("hlt");
}
