#include <debug/debug.h>
#include <graphics/printf.h>

void kmain() {
    serial_init();
    log_printf(LOG_DEBUG, "Debug log!!");
    log_printf(LOG_INFO,  "Info  log!!");
    log_printf(LOG_WARN,  "Warn  log!!");
    log_printf(LOG_ERROR, "Error log!!");
    log_printf(LOG_PANIC, "Panic log!!");
    printf("Hello, world!");
}