#include <stddef.h>
#include <graphics/printf.h>
#include "isr.h"
#include "idt.h"
#include <io/io.h>
#include <debug/debug.h>

ISRHandler g_ISRHandlers[256];

static const char* const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

void ISR_RegisterGates();

void ISR_Initialize() {
    ISR_RegisterGates();
    for (int i = 0; i < 256; i++) {
        IDT_EnableGate(i);
    }
}

void __attribute__((cdecl)) ISR_Handler(Registers* regs)
{
    if (g_ISRHandlers[regs->interrupt] != NULL) {
        g_ISRHandlers[regs->interrupt](regs);
    }

    else if (regs->interrupt >= 32)
        log_printf(LOG_WARN, "Unhandled interrupt 0x%02X!", regs->interrupt);

    else 
    {
        log_printf(LOG_PANIC, "Unhandled exception %d %s", regs->interrupt, g_Exceptions[regs->interrupt]);
        printf("Unhandled exception %d %s!\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        
        log_printf(LOG_PANIC, "  eax=0x%08X  ebx=0x%08X  ecx=0x%08X  edx=0x%08X  esi=0x%08X  edi=0x%08X",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        log_printf(LOG_PANIC, "  esp=0x%08X  ebp=0x%08X  eip=0x%08X  eflags=0x%08X  cs=0x%08X  ds=0x%08X  ss=0x%08X",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

        log_printf(LOG_PANIC, "  interrupt=0x%04X  errorcode=0x%04X", regs->interrupt, regs->error);

        log_printf(LOG_PANIC, "KERNEL PANIC!");
        printf("KERNEL PANIC!");
        Panic();
    }
}

void ISR_RegisterHandler(int interrupt, ISRHandler handler) {
    g_ISRHandlers[interrupt] = handler;
}
