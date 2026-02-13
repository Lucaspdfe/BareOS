#include "../debug/debug.h"
#include "../graphics/printf.h"
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"

void Trigger() {
    printf("int 0x80 triggered!!!");
}

void kmain() {
    GDT_Initialize();
    IDT_Initialize();
    ISR_Initialize();
    ISR_RegisterHandler(0x80, Trigger);
    SERIAL_Initialize();
    log_printf(LOG_DEBUG, "Debug log!!");
    log_printf(LOG_INFO,  "Info  log!!");
    log_printf(LOG_WARN,  "Warn  log!!");
    log_printf(LOG_ERROR, "Error log!!");
    log_printf(LOG_PANIC, "Panic log!!");
    printf("Hello, world!\n");
}