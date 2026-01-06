#pragma once

#include <stdint.h>
#include "isr.h"

typedef void (*Task)(void);

extern volatile uint32_t preempt_disable;

#define PREEMPT_DISABLE() preempt_disable++
#define PREEMPT_ENABLE()  preempt_disable--

void i686_SCHED_Initialize();
void i686_SCHED_Schedule(Registers* regs);
void i686_SCHED_Exit(Registers* regs);
int i686_SCHED_AddTask(Task task);
