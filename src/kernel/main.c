#include <stdint.h>
#include <stdio.h>
#include <hal/hal.h>
#include <hal/vfs.h>

#define PROGRAM_LOAD_ADDR  ((uint32_t)0x00200000)
#define PROGRAM_STACK_TOP  ((uint32_t)0x0020F000)

#define PROGRAM_MAX_SIZE 0x10000

#define USER_CS 0x1B   // 0x18 | 3
#define USER_DS 0x23   // 0x20 | 3

size_t LoadProgram(const char* path) {
    int fd = VFS_Open(path);
    if (fd < 0)
        return 0;

    size_t loaded = VFS_Read(fd, (void*)PROGRAM_LOAD_ADDR, PROGRAM_MAX_SIZE);
    VFS_Close(fd);
    return loaded;
}

struct kernelReturnFrame {
    uint32_t saved_ebp, saved_eip;
};

struct kernelReturnFrame savedFrame;

void JumpToProgram(uint32_t addr, uint32_t stack) {
    __asm__ volatile("movl %%ebp, %0" : "=r"(savedFrame.saved_ebp));
    __asm__ volatile("movl 4(%%ebp), %0" : "=r"(savedFrame.saved_eip));

    __asm__ volatile (
        "mov $0x23, %%ax      \n"
        "mov %%ax, %%ds       \n"
        "mov %%ax, %%es       \n"
        "mov %%ax, %%fs       \n"
        "mov %%ax, %%gs       \n"

        "mov %0, %%eax        \n"   // user ESP
        "pushl $0x23          \n"   // SS
        "pushl %%eax          \n"   // ESP
        "pushfl               \n"   // EFLAGS
        "pushl $0x1B          \n"   // CS
        "pushl %1             \n"   // EIP
        "iret                 \n"
        :
        : "r"(stack), "r"(addr)
        : "eax"
    );
}

void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);

    size_t size = LoadProgram("/usr/prog.bin");
    if (size == 0)
        for (;;) {}

    JumpToProgram(PROGRAM_LOAD_ADDR, PROGRAM_STACK_TOP);

    size = LoadProgram("/usr/prog2.bin");
    if (size == 0)
        for (;;) {}

    JumpToProgram(PROGRAM_LOAD_ADDR, PROGRAM_STACK_TOP);

    printf("Returned!!!!");

    for (;;) {}
}
