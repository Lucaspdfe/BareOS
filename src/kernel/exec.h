#pragma once

#include <stdint.h>
#include <stddef.h>

struct kernelReturnFrame {
    uint32_t saved_ebp, saved_eip;
};

extern struct kernelReturnFrame savedFrame;

size_t LoadProgram(const char* path);
uint32_t PrepareUserStack(uint32_t stack_top, char* argv[], int argc);
void JumpToProgram(uint32_t addr, uint32_t stack);
void JumpToProgramNotReturn(uint32_t addr, uint32_t stack);
