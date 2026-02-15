#include "../debug/debug.h"
#include "../graphics/printf.h"
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/pic.h"
#include "../cpu/irq.h"
#include "../cpu/pit.h"
#include "../cpu/id.h"
#include "../cpu/cmos.h"
#include "../cpu/acpi.h"
#include <stdbool.h>

void kmain() {
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