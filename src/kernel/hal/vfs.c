#include "vfs.h"
#include <arch/i686/fat.h>
#include <arch/i686/key.h>
#include <arch/i686/disp.h>
#include <arch/i686/debug.h>
#include <string.h>

VFS_FileDescriptor devices[255];
VFS_FATInfo file[8];

ssize_t VFS_FD0Read(VFS_FileDescriptor* vfd, char* buffer, size_t size) {
    size_t i = 0;
    while (i < size) {
        KEYState state = i686_KEY_WaitKey();
        if (state.isSpecial) {
            if (state.specialKey == KEY_CAPS) continue;
            if (state.specialKey == KEY_ARROW_LEFT || state.specialKey == KEY_ARROW_RIGHT ||
                state.specialKey == KEY_ARROW_UP   || state.specialKey == KEY_ARROW_DOWN) {
                continue;
            }
            continue;
        }
        char c = state.character;
        if (c == '\0') continue;
        if (c == '\b') {
            if (i > 0) {
                i--;
                i686_DISP_PutChar('\b');
            }
            continue;
        }
        i686_DISP_PutChar(c);
        if (c == '\n') break;
        buffer[i++] = c;
    }
    return i;
}

ssize_t VFS_FD1Write(VFS_FileDescriptor* vfd, const char* buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        i686_DISP_PutChar(buffer[i]);
    }
    return size;
}

ssize_t VFS_FD2Write(VFS_FileDescriptor* vfd, const char* buffer, size_t size) {
    i686_DEBUG_Debugs("\033[31m[ERR] ");
    for (size_t i = 0; i < size; i++) {
        i686_DEBUG_Debugc(buffer[i]);
    }
    i686_DEBUG_Debugs("\033[0m\n");
    return size;
}

ssize_t VFS_FATWrite(VFS_FileDescriptor* vfd, const char* buffer, size_t size) {
    VFS_FATInfo* info = vfd->info;

    return i686_FAT_Write(info->fd, buffer, size);
}

ssize_t VFS_FATRead(VFS_FileDescriptor* vfd, char* buffer, size_t size) {
    VFS_FATInfo* info = vfd->info;

    return i686_FAT_Read(info->fd, buffer, size);
}

int VFS_FATClose(VFS_FileDescriptor* vfd) {
    VFS_FATInfo* info = vfd->info;

    i686_FAT_Close(info->fd);

    vfd->isOpen = false;
}

void VFS_Initialize() {
    for (int i = 0; i < 255; i++) {
        memset(&devices[i], 0, sizeof(VFS_FileDescriptor));
    }

    // Set standard file descriptors
    devices[0].isOpen = true;
    devices[0].flags  = ATTR_STDFD | ATTR_DEVICE;
    devices[0].info   = (void*)0;
    devices[0].close  = (void*)0;
    devices[0].write  = (void*)0;
    devices[0].read   = VFS_FD0Read;

    devices[1].isOpen = true;
    devices[1].flags  = ATTR_STDFD | ATTR_DEVICE;
    devices[1].info   = (void*)0;
    devices[1].close  = (void*)0;
    devices[1].write  = VFS_FD1Write;
    devices[1].read   = (void*)0;

    devices[2].isOpen = true;
    devices[2].flags  = ATTR_STDFD | ATTR_DEVICE;
    devices[2].info   = (void*)0;
    devices[2].close  = (void*)0;
    devices[2].write  = VFS_FD2Write;
    devices[2].read   = (void*)0;
}

/* =========================
   WRITE
   ========================= */
ssize_t VFS_Write(int fd, const char* buffer, size_t amount) {
    if (fd > 255) return -EBADF;
    if (!devices[fd].isOpen) return -EBADF;
    if (devices[fd].write == (void*)0) return -EBADF;

    return devices[fd].write(&devices[fd], buffer, amount);
}

/* =========================
   READ
   ========================= */
ssize_t VFS_Read(int fd, char* buffer, size_t amount) {
    if (fd > 255) return -EBADF;
    if (!devices[fd].isOpen) return -EBADF;
    if (devices[fd].read == (void*)0) return -EBADF;

    return devices[fd].read(&devices[fd], buffer, amount);
}

int VFS_Open(const char* path) {
    for (int i = 0; i < 255; i++) {
        if (!devices[i].isOpen) {
            int ret = i686_FAT_Open(path);
            if (ret < 0) {
                if (ret == -1) return -EMFILE;
                if (ret == -2) return -ENOENT;
                if (ret == -3) return -ENOTDIR;
            }
            file[ret].fd      = ret;
            devices[i].info   = &file[ret];
            devices[i].isOpen = true;
            devices[i].flags  = ATTR_FAT;
            devices[i].close  = VFS_FATClose;
            devices[i].read   = VFS_FATRead;
            devices[i].write  = VFS_FATWrite;
            return i;
        }
    }
    return -EMFILE;
}

int VFS_Close(int fd) {
    if (fd > 255) return -EBADF;
    if (!devices[fd].isOpen) return -EBADF;
    if (devices[fd].close == (void*)0) return -EBADF;

    return devices[fd].close(&devices[fd]);
}
