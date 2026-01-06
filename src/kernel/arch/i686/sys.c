#include <stdint.h>
#include <string.h>
#include "pit.h"
#include "sys.h"
#include "isr.h"
#include "fat.h"
#include "io.h"
#include "debug.h"
#include "sched.h"
#include <hal/vfs.h>

enum { 
    SYS_EXIT = 1, 
    SYS_READ = 3, 
    SYS_WRITE = 4, 
    SYS_OPEN = 5, 
    SYS_CLOSE = 6,
    SYS_GETDENTS = 141,
    SYS_NANOSLEEP = 162,
};

struct linux_timespec {
    uint32_t tv_sec;  // seconds
    uint32_t tv_nsec; // nanoseconds
};

struct linux_dirent {
    uint32_t d_ino;         // inode number
    uint32_t d_off;         // offset to next dirent
    uint16_t d_reclen;      // size of entry
    char     d_name[];      // null-terminated filename
};

enum linux_dirent_type {
    DT_REG     = 8,
    DT_DIR     = 4,
    DT_LNK     = 10,
    DT_UNKNOWN = 0,
};

void i686_SYS_Handler(Registers* regs) {
    PREEMPT_DISABLE();
    i686_EnableInterrupts();
    uint32_t syscall = regs->eax;

    switch (syscall) {
        case SYS_EXIT: {
            i686_SCHED_Exit(regs);
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

        case SYS_GETDENTS: {
            int fd = regs->ebx - 3;     // taking into account stdin, out and err
            struct linux_dirent* dirp = (struct linux_dirent*)regs->ecx;
            unsigned int count = regs->edx;

            FAT_File* dir = i686_FAT_GetFileInfo(fd);
            if (!dir || !(dir->entry.Attributes & ATTR_DIRECTORY)) {
                regs->eax = (uint32_t)-1;
                break;
            }

            uint32_t index = dir->currentOffset; // use currentOffset as directory entry index
            uint32_t bytes_written = 0;
            char namebuf[16];

            while (1) {
                FAT_DirectoryEntry entry;
                if (!i686_FAT_ReadEntry(fd, index, &entry))
                    break; // no more entries

                if (entry.Attributes == 0x0F) {
                    index++;
                    continue;
                }

                i686_FAT_ExtractName(entry.FileName, namebuf);
                size_t namelen = strlen(namebuf);

                /* Layout: d_ino (4) | d_off (4) | d_reclen (2) | d_name (namelen+1) ... | type (1 at last byte)
                   minimal reclen = 10 + namelen + 1 (type). Align to 4 bytes as in linux getdents. */
                uint16_t reclen = (uint16_t)(10 + namelen + 1);
                uint16_t padded = (reclen + 3) & ~3;

                if (bytes_written + padded > count)
                    break; // not enough space in user buffer

                char* dest = (char*)dirp + bytes_written;

                // d_ino: simple inode number (use index+1)
                *(uint32_t*)(dest + 0) = index + 1;
                // d_off: next entry index (userland may use this to continue)
                *(uint32_t*)(dest + 4) = index + 1;
                // d_reclen
                *(uint16_t*)(dest + 8) = padded;

                // d_name (null-terminated)
                char* namedst = dest + 10;
                memcpy(namedst, namebuf, namelen);
                namedst[namelen] = '\0';

                // file type stored in last byte of the record when it doesn't
                // collide with the name's NUL terminator. Preserve the NUL
                // so userspace `strlen` works reliably.
                unsigned char type = DT_REG;
                if (entry.Attributes & ATTR_DIRECTORY)
                    type = DT_DIR;
                size_t name_term_offset = 10 + namelen;
                if ((int)(padded - 1) != (int)name_term_offset) {
                    dest[padded - 1] = type;
                } else {
                    dest[name_term_offset] = '\0';
                }

                bytes_written += padded;
                index++;
            }

            dir->currentOffset = index;
            regs->eax = bytes_written;
            break;
        }

        case SYS_NANOSLEEP: {
            struct linux_timespec* req = (struct linux_timespec*)regs->ebx;

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
    i686_DisableInterrupts();
    PREEMPT_ENABLE();
}

void i686_SYS_Initialize() {
    i686_ISR_RegisterHandler(0x80, i686_SYS_Handler);
}
