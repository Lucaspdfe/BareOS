#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2

#define ENOENT  2
#define EIO     5
#define EBADF   9
#define ENOTDIR 20
#define EMFILE  24

#define ATTR_FAT    0x01
#define ATTR_DEVICE 0x02
#define ATTR_STDFD  0x04            // standard file descriptors. cannot be closed

typedef struct VFS_FileDescriptor VFS_FileDescriptor;
typedef signed long ssize_t;

struct VFS_FileDescriptor {
    bool    isOpen;
    uint8_t flags;
    void*   info;
    ssize_t (*read)(VFS_FileDescriptor*, char*, size_t);
    ssize_t (*write)(VFS_FileDescriptor*, const char*, size_t);
    int     (*close)(VFS_FileDescriptor*);
};

typedef struct {
    int fd;
} VFS_FATInfo;

void VFS_Initialize();
ssize_t VFS_Write(int fd, const char* buffer, size_t amount);
ssize_t VFS_Read(int fd, char* buffer, size_t amount);
int VFS_Open(const char* path);
int VFS_Close(int fd);
