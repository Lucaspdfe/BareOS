#include "isr.h"
#include "irq.h"
#include "pic.h"
#include <debug/debug.h>
#include <io/io.h>
#include <stddef.h>

#define PIC_REMAP_OFFSET        0x20

IRQHandler g_IRQHandlers[16];

void IRQ_Handler(Registers* regs)
{
    int irq = regs->interrupt - PIC_REMAP_OFFSET;
    
    uint8_t pic_isr = PIC_ReadInServiceRegister();
    uint8_t pic_irr = PIC_ReadIrqRequestRegister();

    if (g_IRQHandlers[irq] != NULL)
    {
        // handle IRQ
        g_IRQHandlers[irq](regs);
    }
    else
    {
        log_printf(LOG_WARN, "Unhandled IRQ %d  ISR=0x%02X  IRR=%02X...", irq, pic_isr, pic_irr);
    }

    // send EOI
    PIC_SendEndOfInterrupt(irq);
}

void IRQ_Initialize()
{
    PIC_Configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8);

    // register ISR handlers for each of the 16 irq lines
    for (int i = 0; i < 16; i++)
        ISR_RegisterHandler(PIC_REMAP_OFFSET + i, IRQ_Handler);

    // enable interrupts
    EnableInterrupts();
}

void IRQ_RegisterHandler(int irq, IRQHandler handler)
{
    g_IRQHandlers[irq] = handler;
}
typedef void (*IRQHandler)(Registers* regs);

void IRQ_Initialize();
void IRQ_RegisterHandler(int irq, IRQHandler handler);