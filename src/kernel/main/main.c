#include "../debug/debug.h"
#include "../graphics/printf.h"
#include "../cpu/gdt.h"

void kmain() {
    GDT_Initialize();
    SERIAL_Initialize();
    log_printf(LOG_DEBUG, "Debug log!!");
    log_printf(LOG_INFO,  "Info  log!!");
    log_printf(LOG_WARN,  "Warn  log!!");
    log_printf(LOG_ERROR, "Error log!!");
    log_printf(LOG_PANIC, "Panic log!!");
    printf("Hello, world!");
}