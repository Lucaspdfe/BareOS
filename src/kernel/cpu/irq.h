#pragma once
#include "isr.h"

typedef void (*IRQHandler)(Registers* regs);

void IRQ_Initialize();
void IRQ_RegisterHandler(int irq, IRQHandler handler);