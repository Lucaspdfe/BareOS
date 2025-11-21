#include <stdint.h>
#include "pit.h"
#include "sys.h"
#include "isr.h"
#include <hal/vfs.h>

struct kernelReturnFrame {
    uint32_t saved_ebp, saved_eip;
};

extern struct kernelReturnFrame savedFrame;

enum { 
    SYS_EXIT = 1,
    SYS_READ = 3, 
    SYS_WRITE = 4, 
    SYS_OPEN = 5, 
    SYS_CLOSE = 6, 
    SYS_NANOSLEEP = 162
};

struct timespec {
    uint32_t tv_sec;  // seconds
    uint32_t tv_nsec; // nanoseconds
};

void i686_SYS_Handler(Registers* regs) {
    uint32_t syscall = regs->eax;

    switch (syscall) {
        case SYS_EXIT: {
            int exitcode = regs->ebx;
            __asm__ volatile("movl %0, %%ebp  \n"
                             "pushl %1        \n"
                             "ret             \n"

                             :
                             : "r"(savedFrame.saved_ebp),
                               "r"(savedFrame.saved_eip));
            break;
        }

        case SYS_READ: {
            int fd = regs->ebx;
            char* buf = (char*)regs->ecx;
            size_t size = regs->edx;
            regs->eax = VFS_Read(fd, buf, size);
            break;
        }

        case SYS_WRITE: {
            int fd = regs->ebx;
            const char* buf = (const char*)regs->ecx;
            size_t size = regs->edx;
            regs->eax = VFS_Write(fd, buf, size);
            break;
        }

        case SYS_OPEN: {
            const char* path = (const char*)regs->ebx;
            regs->eax = VFS_Open(path);
            break;
        }

        case SYS_CLOSE: {
            int fd = regs->ebx;
            VFS_Close(fd);
            regs->eax = 0;
            break;
        }

        case SYS_NANOSLEEP: {
            struct timespec* req = (struct timespec*)regs->ebx;

            if (req) {
                uint32_t ms = req->tv_sec * 1000 + req->tv_nsec / 1000000;
                i686_PIT_Sleep(ms);
            }

            regs->eax = 0;
            break;
        }

        default:
            regs->eax = (uint32_t)-1;
            break;
    }
}

void i686_SYS_Initialize() {
    i686_ISR_RegisterHandler(0x80, i686_SYS_Handler);
}
