#pragma once
#include <stdint.h>
#include <stddef.h>

#define STDIN  0
#define STDOUT 1
#define STDERR 2

size_t VFS_Write(int fd, const char* buffer, size_t amount);
size_t VFS_Read(int fd, char* buffer, size_t amount);
int VFS_Open(const char* path);
void VFS_Close(int fd);
