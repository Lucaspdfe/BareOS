#include <stddef.h>
#include <stdio.h>
#include "isr.h"
#include "idt.h"
#include "io.h"
#include "debug.h"

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

void i686_ISR_InitializeGates();

void i686_ISR_Initialize() {
    i686_ISR_InitializeGates();
    for (int i = 0; i < 256; i++) {
        i686_IDT_EnableGate(i);
    }
}

void __attribute__((cdecl)) i686_ISR_Handler(Registers* regs)
{
    if (g_ISRHandlers[regs->interrupt] != NULL) {
        i686_EnableInterrupts();
        g_ISRHandlers[regs->interrupt](regs);
        i686_DisableInterrupts();
    }

    else if (regs->interrupt >= 32)
        i686_DEBUG_Debugf(LOG_WARN, "Unhandled interrupt %d!", regs->interrupt);

    else 
    {
        i686_DEBUG_Debugf(LOG_CRIT, "Unhandled exception %d %s", regs->interrupt, g_Exceptions[regs->interrupt]);
        printf("Unhandled exception %d %s!\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        
        i686_DEBUG_Debugf(LOG_CRIT, "  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        i686_DEBUG_Debugf(LOG_CRIT, "  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

        i686_DEBUG_Debugf(LOG_CRIT, "  interrupt=%x  errorcode=%x", regs->interrupt, regs->error);

        i686_DEBUG_Debugf(LOG_CRIT, "KERNEL PANIC!");
        printf("KERNEL PANIC!");
        i686_Panic();
    }
}

void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler) {
    g_ISRHandlers[interrupt] = handler;
}
