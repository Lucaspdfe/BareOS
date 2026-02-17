#include "../debug/debug.h"
#include "../graphics/printf.h"
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/pic.h"
#include "../cpu/irq.h"
#include "../cpu/pit.h"
#include "../cpu/id.h"
#include "../disk/disk.h"
#include "../cpu/cmos.h"
#include "../cpu/acpi.h"
#include <stdbool.h>

void kmain(uint8_t bootDrive) {
    printf("Hello, world!\n");
    GDT_Initialize();
    IDT_Initialize();
    ISR_Initialize();
    IRQ_Initialize();
    PIT_Initialize();
    ACPI_Initialize();
    SERIAL_Initialize();

    bool cpuid = ID_CheckSupported();
    log_printf(cpuid ? LOG_INFO : LOG_WARN, "CPUID Support: %s", cpuid ? "True" : "False");

    bool apic = ID_APICSupport();
    log_printf(apic ? LOG_INFO : LOG_WARN, "APIC Support: %s", apic ? "True" : "False");

    char buffer[256]; ID_CPUName(buffer);
    log_printf(LOG_INFO, "CPU Name: %s", buffer);

    uint32_t sec, min, hour, day, month, year;
    CMOS_RTCRead(&sec, &min, &hour, &day, &month, &year);
    if (hour > 12) hour -= 12;
    log_printf(LOG_INFO, "Time: %02d:%02d:%02d, %02d/%02d/%04d", hour, min, sec, day, month, year);

    uint8_t floppy0, floppy1;
    CMOS_GetFloppyType(&floppy0, &floppy1);
    const char* floppyTypes[] = {"None", "360KB", "1.2MB", "720KB", "1.44MB", "2.88MB"};
    log_printf(LOG_INFO, "Floppy Drive 0: %s", floppyTypes[floppy0]);
    log_printf(LOG_INFO, "Floppy Drive 1: %s", floppyTypes[floppy1]);
    log_printf(LOG_INFO, "Boot Drive: 0x%02X", bootDrive);

    DISK disk;
    DISK_Initialize(bootDrive, &disk);
    log_printf(LOG_INFO, "Disk %d: %d Cylinders, %d Heads, %d Sectors", disk.driveNumber, disk.maxCylinders, disk.maxHeads, disk.maxSectors);
    uint8_t buffer512[512];
    DISK_Seek(&disk, 91);
    DISK_ReadSectors(&disk, 1, buffer512);
    log_printf(LOG_INFO, "First sector of disk %d: %02X %02X %02X %02X ...", disk.driveNumber, buffer512[0], buffer512[1], buffer512[2], buffer512[3]);

    printf("Shutting off in 5...");
    SERIAL_Write("[INFO ] Shutting off in 5...");
    PIT_Sleep(1000);
    printf("\b\b\b\b4...");
    SERIAL_Write("\b\b\b\b4...");
    PIT_Sleep(1000);
    printf("\b\b\b\b3...");
    SERIAL_Write("\b\b\b\b3...");   
    PIT_Sleep(1000);
    printf("\b\b\b\b2...");
    SERIAL_Write("\b\b\b\b2...");
    PIT_Sleep(1000);
    printf("\b\b\b\b1...\n");
    SERIAL_Write("\b\b\b\b1...");
    PIT_Sleep(1000);
    SERIAL_Putc('\n');
    ACPI_PowerOff();
}