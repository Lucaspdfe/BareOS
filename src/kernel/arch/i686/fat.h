#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "disk.h"

typedef struct {
    char FileName[11];
    uint8_t Attributes;
    uint8_t _Reserved;
    uint8_t CreationTime100ths;
    uint16_t CreationTime;
    uint16_t CreationDate;
    uint16_t AccessDate;
    uint16_t HighFirstCluster;
    uint16_t ModifiedTime;
    uint16_t ModifiedDate;
    uint16_t LowFirstCluster;
    uint32_t Size;
} __attribute__((packed)) FAT_DirectoryEntry;

typedef struct FAT_File {
    FAT_DirectoryEntry entry;
    uint32_t currentCluster;
    uint32_t currentOffset;
    bool open;
} FAT_File;

#define ATTR_DIRECTORY 0x10

// Public API
FAT_File* i686_FAT_GetFileInfo(int fd);
void i686_FAT_ExtractName(const char fatName[11], char* out);
bool i686_FAT_Initialize(DISK* disk);
int i686_FAT_Open(const char* path);
uint32_t i686_FAT_Read(int fd, void* buffer, uint32_t size);
bool i686_FAT_ReadEntry(int fd, uint32_t index, FAT_DirectoryEntry* outEntry);
uint32_t i686_FAT_Write(int fd, const void* buffer, uint32_t size);
bool FAT_UpdateDirectoryEntry(uint32_t startCluster, uint32_t entryIndex, FAT_DirectoryEntry* entry);
int i686_FAT_CreateFile(const char* path);
void i686_FAT_Close(int fd);