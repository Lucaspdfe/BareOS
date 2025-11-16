#include "sys.h"
#include "isr.h"
#include <hal/vfs.h>

enum {
    SYS_READ  = 0,
    SYS_WRITE = 1,
    SYS_OPEN  = 2,
    SYS_CLOSE = 3
};

void i686_SYS_Handler(Registers* regs) {
    uint32_t syscall = regs->eax;

    switch (syscall) {
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

        default:
            regs->eax = (uint32_t)-1;
            break;
    }
}

void i686_SYS_Initialize() {
    i686_ISR_RegisterHandler(0x80, i686_SYS_Handler);
}
