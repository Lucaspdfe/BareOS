#include "sched.h"
#include "alloc.h"
#include <string.h>
#include <stdio.h>

Registers tasks[255];
uint8_t current;

volatile uint32_t preempt_disable = 0;

void idle() {
    for (;;) {
        __asm__ volatile ("hlt"); 
    }
}

void i686_SCHED_Initialize() {
    memset(tasks, 0, sizeof(tasks));

    tasks[0].eip = (uint32_t)idle;
    tasks[0].cs  = 0x08;
    tasks[0].ds  = 0x10;
    tasks[0].ss  = 0x10;
    tasks[0].ebp = (uint32_t)(i686_ALO_Malloc(4096)) + 4096;
    tasks[0].esp = tasks[0].ebp;
    tasks[0].eflags = 0x202;
    current = 0;
}

void i686_SCHED_Schedule(Registers* regs) {
    /* Save current task state */
    tasks[current] = *regs;

    /* Find next runnable task */
    uint8_t next = current;
    for (int i = 0; i < 255; i++) {
        next = (next + 1) % 255;
        if (tasks[next].eip != 0) {
            break;
        }
    }

    current = next;

    /* Load next task state */
    *regs = tasks[current];
}

void i686_SCHED_Exit(Registers* regs) {
    /* Kill current task */
    memset(&tasks[current], 0, sizeof(Registers));

    /* Pick next runnable task */
    uint8_t next = current;
    for (int i = 0; i < 255; i++) {
        next = (next + 1) % 255;
        if (tasks[next].eip != 0) {
            current = next;
            *regs = tasks[current];
            return;
        }
    }

    /* Fallback: idle must exist */
    current = 0;
    *regs = tasks[0];
}

int i686_SCHED_AddTask(Task task) {
    for (int i = 0; i < 255; i++) {
        if (tasks[i].eip == 0) {
            tasks[i].eip = (uint32_t)task;
            tasks[i].cs  = 0x08;
            tasks[i].ds  = 0x10;
            tasks[i].ss  = 0x10;
            tasks[i].ebp = (uint32_t)(i686_ALO_Malloc(4096)) + 4096;
            tasks[i].esp = tasks[0].ebp;
            tasks[i].eflags = 0x202;
            return i;
        }
    }

    return -1;
}
