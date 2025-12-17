#include "exec.h"
#include <hal/vfs.h>
#include <string.h>
#include <stdint.h>

struct kernelReturnFrame savedFrame;

#define PROGRAM_LOAD_ADDR  ((uint32_t)0x00200000)
#define PROGRAM_MAX_SIZE 0x10000

size_t LoadProgram(const char* path) {
    int fd = VFS_Open(path);
    if (fd < 0)
        return 0;

    size_t loaded = VFS_Read(fd, (void*)PROGRAM_LOAD_ADDR, PROGRAM_MAX_SIZE);
    VFS_Close(fd);
    return loaded;
}

uint32_t PrepareUserStack(uint32_t stack_top, char* argv[], int argc) {
    if (argc < 0) argc = 0;
    if (argc > 16) argc = 16;

    uint32_t sp = stack_top;
    char* user_ptrs[16];

    // copy strings (from last to first)
    for (int i = argc - 1; i >= 0; --i) {
        size_t len = strlen(argv[i]) + 1;
        sp -= len;
        // align to 4 bytes
        sp &= ~0x3u;
        memcpy((void*)sp, argv[i], len);
        user_ptrs[i] = (char*)sp;
    }

    // Reserve space for: argc (1) + argv pointers (argc) + NULL (1) + envp NULL (1)
    uint32_t total_words = 1 + argc + 1 + 1;
    uint32_t bytes = total_words * 4;
    uint32_t sp_ptrs = (sp - bytes) & ~0x3u;

    // write argc at lowest address
    *(uint32_t*)(sp_ptrs) = (uint32_t)argc;

    // write argv pointers
    for (int i = 0; i < argc; ++i) {
        *(uint32_t*)(sp_ptrs + 4 + i * 4) = (uint32_t)user_ptrs[i];
    }

    // argv NULL
    *(uint32_t*)(sp_ptrs + 4 + argc * 4) = 0;
    // envp NULL
    *(uint32_t*)(sp_ptrs + 4 + (argc + 1) * 4) = 0;

    return sp_ptrs;
}

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

void JumpToProgramNotReturn(uint32_t addr, uint32_t stack) {
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
