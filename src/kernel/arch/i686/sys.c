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
    SYS_EXIT       = 1,
    SYS_READ       = 3,
    SYS_WRITE      = 4,
    SYS_OPEN       = 5,
    SYS_CLOSE      = 6,
    SYS_GETDENTS   = 141,
    SYS_NANOSLEEP  = 162,
    SYS_GETDENTS64 = 220,
};

struct linux_timespec {
    uint32_t tv_sec;
    uint32_t tv_nsec;
};

/* Linux-compatible getdents layout:
 * d_type is stored in the LAST byte of the record
 */
struct linux_dirent {
    uint32_t d_ino;
    uint32_t d_off;
    uint16_t d_reclen;
    char     d_name[];
};

struct linux_dirent64 {
    uint64_t d_ino;
    int64_t  d_off;
    uint16_t d_reclen;
    uint8_t  d_type;
    char     d_name[];
};

enum linux_dirent_type {
    DT_UNKNOWN = 0,
    DT_DIR     = 4,
    DT_REG     = 8,
    DT_LNK     = 10,
};

void i686_SYS_Handler(Registers* regs) {
    PREEMPT_DISABLE();
    i686_EnableInterrupts();

    uint32_t syscall = regs->eax;

    switch (syscall) {

        case SYS_EXIT:
            i686_SCHED_Exit(regs);
            break;

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
            regs->eax = VFS_Close(fd);
            break;
        }

        case SYS_GETDENTS: {
            int fd = regs->ebx - 3; /* skip stdin/stdout/stderr */
            struct linux_dirent* dirp = (struct linux_dirent*)regs->ecx;
            unsigned int count = regs->edx;

            FAT_File* dir = i686_FAT_GetFileInfo(fd);
            if (!dir || !(dir->entry.Attributes & ATTR_DIRECTORY)) {
                regs->eax = (uint32_t)-1;
                break;
            }

            uint32_t index = dir->currentOffset;
            uint32_t bytes_written = 0;
            char namebuf[16];

            while (1) {
                FAT_DirectoryEntry entry;
                if (!i686_FAT_ReadEntry(fd, index, &entry))
                    break;

                /* skip LFN entries */
                if (entry.Attributes == 0x0F) {
                    index++;
                    continue;
                }

                /* skip volume label (drive name) */
                if (entry.Attributes & ATTR_VOLUME_ID) {
                    index++;
                    continue;
                }


                i686_FAT_ExtractName(entry.FileName, namebuf);
                size_t namelen = strlen(namebuf);

                /* header (10) + name + NUL + d_type */
                uint16_t reclen = 10 + namelen + 1 + 1;
                uint16_t padded = (reclen + 3) & ~3;

                if (bytes_written + padded > count)
                    break;

                char* dest = (char*)dirp + bytes_written;

                /* d_ino */
                *(uint32_t*)(dest + 0) = index + 1;

                /* d_off */
                *(uint32_t*)(dest + 4) = index + 1;

                /* d_reclen */
                *(uint16_t*)(dest + 8) = padded;

                /* d_name */
                char* namedst = dest + 10;
                memcpy(namedst, namebuf, namelen);
                namedst[namelen] = '\0';

                /* d_type: ALWAYS last byte */
                unsigned char type = DT_REG;
                if (entry.Attributes & ATTR_DIRECTORY)
                    type = DT_DIR;

                dest[padded - 1] = type;

                bytes_written += padded;
                index++;
            }

            dir->currentOffset = index;
            regs->eax = bytes_written;
            break;
        }

        case SYS_NANOSLEEP: {
            struct linux_timespec* req =
                (struct linux_timespec*)regs->ebx;

            if (req) {
                uint32_t ms =
                    req->tv_sec * 1000 +
                    req->tv_nsec / 1000000;
                i686_PIT_Sleep(ms);
            }

            regs->eax = 0;
            break;
        }

        case SYS_GETDENTS64: {
            int fd = regs->ebx - 3;
            struct linux_dirent64* dirp =
                (struct linux_dirent64*)regs->ecx;
            unsigned int count = regs->edx;

            FAT_File* dir = i686_FAT_GetFileInfo(fd);
            if (!dir || !(dir->entry.Attributes & ATTR_DIRECTORY)) {
                regs->eax = (uint32_t)-1;
                break;
            }
        
            uint32_t index = dir->currentOffset;
            uint32_t bytes_written = 0;
            char namebuf[16];
        
            while (1) {
                FAT_DirectoryEntry entry;
                if (!i686_FAT_ReadEntry(fd, index, &entry))
                    break;
            
                /* skip LFN entries */
                if (entry.Attributes == 0x0F) {
                    index++;
                    continue;
                }

                /* skip volume label (drive name) */
                if (entry.Attributes & ATTR_VOLUME_ID) {
                    index++;
                    continue;
                }
            
                i686_FAT_ExtractName(entry.FileName, namebuf);
                size_t namelen = strlen(namebuf);
            
                uint16_t reclen =
                    sizeof(struct linux_dirent64) + namelen + 1;
            
                reclen = (reclen + 7) & ~7; /* 8-byte align */
            
                if (bytes_written + reclen > count)
                    break;
            
                struct linux_dirent64* d =
                    (struct linux_dirent64*)((char*)dirp + bytes_written);
            
                d->d_ino  = (uint64_t)(index + 1);
                d->d_off  = (int64_t)(index + 1);
                d->d_reclen = reclen;
            
                if (entry.Attributes & ATTR_DIRECTORY)
                    d->d_type = DT_DIR;
                else
                    d->d_type = DT_REG;
            
                memcpy(d->d_name, namebuf, namelen);
                d->d_name[namelen] = '\0';
            
                bytes_written += reclen;
                index++;
            }
        
            dir->currentOffset = index;
            regs->eax = bytes_written;
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
