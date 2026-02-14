#include "../debug/debug.h"
#include "../graphics/printf.h"
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/id.h"
#include "../cpu/cmos.h"
#include <stdbool.h>

void Trigger() {
    printf("int 0x80 triggered!!!");
}

void kmain() {
    GDT_Initialize();
    IDT_Initialize();
    ISR_Initialize();
    ISR_RegisterHandler(0x80, Trigger);
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

    printf("Hello, world!\n");
    __asm("int $0x80");
}