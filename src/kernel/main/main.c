#include "../debug/debug.h"
#include "../graphics/printf.h"
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/pic.h"
#include "../cpu/irq.h"
#include "../cpu/id.h"
#include "../cpu/cmos.h"
#include <stdbool.h>

void timer() {
    putc('.');
}

void Int0x80() {
    puts("Int 0x80 Triggered!\n");
}

void kmain() {
    printf("Hello, world!\n");
    GDT_Initialize();
    IDT_Initialize();
    ISR_Initialize();
    IRQ_Initialize();
    SERIAL_Initialize();

    ISR_RegisterHandler(0x80, Int0x80);
    __asm("int $0x80");

    IRQ_RegisterHandler(0, timer);

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

}