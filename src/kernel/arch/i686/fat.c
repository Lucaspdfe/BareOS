#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "fat.h"
#include "disk.h"
#include "debug.h"

#define MAX_OPEN_FILES 8

typedef struct FAT_BootSector {
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;
    uint8_t VolumeLabel[11];
    uint8_t SystemId[8];
} __attribute__((packed)) FAT_BootSector;

static FAT_BootSector g_bootSector;
static uint32_t g_fatStart;
static uint32_t g_rootDirStart;
static uint32_t g_rootDirSectors;
static uint32_t g_dataStart;
static DISK* g_disk;
static FAT_File g_openFiles[MAX_OPEN_FILES];

// -----------------------------------------------------------
// Utilities
// -----------------------------------------------------------

static bool match_filename(const char* fatname, const char* target) {
    return memcmp(fatname, target, 11) == 0;
}

static void to_fat_name(const char* input, char* output) {
    memset(output, ' ', 11);
    const char* dot = strchr(input, '.');
    int nameLen = dot ? (dot - input) : strlen(input);
    int extLen = dot ? strlen(dot + 1) : 0;

    for (int i = 0; i < nameLen && i < 8; i++) {
        char c = input[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        output[i] = c;
    }
    if (dot) {
        for (int i = 0; i < extLen && i < 3; i++) {
            char c = dot[1 + i];
            if (c >= 'a' && c <= 'z') c -= 32;
            output[8 + i] = c;
        }
    }
}

FAT_File* i686_FAT_GetFileInfo(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        return NULL;
    }

    if (!g_openFiles[fd].open) {
        return NULL;
    }

    return &g_openFiles[fd];
}

void i686_FAT_ExtractName(const char fatName[11], char* out) {
    int i = 0;
    int j = 0;

    // --- convert and copy name part (first 8 chars)
    for (i = 0; i < 8 && fatName[i] != ' '; i++) {
        char c = fatName[i];
        if (c >= 'A' && c <= 'Z') c += 32; // tolower
        out[j++] = c;
    }

    // --- check if extension exists
    if (fatName[8] != ' ') {
        out[j++] = '.';
        for (i = 8; i < 11 && fatName[i] != ' '; i++) {
            char c = fatName[i];
            if (c >= 'A' && c <= 'Z') c += 32; // tolower
            out[j++] = c;
        }
    }

    out[j] = '\0';
}

// -----------------------------------------------------------
// Initialization
// -----------------------------------------------------------

bool i686_FAT_Initialize(DISK* disk) {
    uint8_t sector[512];
    g_disk = disk;

    if (!DISK_ReadSectors(disk, 0, 1, sector))
        return false;

    // Check boot signature 0x55AA
    if (sector[510] != 0x55 || sector[511] != 0xAA)
        return false;

    memcpy(&g_bootSector, sector, sizeof(FAT_BootSector));

    g_fatStart = g_bootSector.ReservedSectors;
    g_rootDirStart = g_fatStart + (g_bootSector.FatCount * g_bootSector.SectorsPerFat);
    g_rootDirSectors =
        ((g_bootSector.DirEntryCount * 32) + (g_bootSector.BytesPerSector - 1)) /
        g_bootSector.BytesPerSector;
    g_dataStart = g_rootDirStart + g_rootDirSectors;

    memset(g_openFiles, 0, sizeof(g_openFiles));
    return true;
}

// -----------------------------------------------------------
// Directory Reading
// -----------------------------------------------------------

static bool FAT_ReadEntryAt(uint32_t sector, uint32_t offset, FAT_DirectoryEntry* outEntry) {
    uint8_t sectorBuf[512];
    if (!DISK_ReadSectors(g_disk, sector, 1, sectorBuf)) return false;
    FAT_DirectoryEntry* entries = (FAT_DirectoryEntry*)sectorBuf;
    memcpy(outEntry, &entries[offset], sizeof(FAT_DirectoryEntry));
    return true;
}

// Reads all entries from a given directory cluster or from root
static bool FAT_FindInDirectory(uint32_t startCluster, const char* fatname, FAT_DirectoryEntry* outEntry) {
    uint8_t sector[512];
    uint32_t entriesPerSector = g_bootSector.BytesPerSector / 32;

    if (startCluster == 0) {
        // Root directory
        for (uint32_t i = 0; i < g_rootDirSectors; i++) {
            if (!DISK_ReadSectors(g_disk, g_rootDirStart + i, 1, sector))
                return false;
            FAT_DirectoryEntry* entries = (FAT_DirectoryEntry*)sector;
            for (uint32_t j = 0; j < entriesPerSector; j++) {
                if (entries[j].FileName[0] == 0x00) return false;
                if ((uint8_t)entries[j].FileName[0] == 0xE5) continue;
                if (match_filename(entries[j].FileName, fatname)) {
                    memcpy(outEntry, &entries[j], sizeof(FAT_DirectoryEntry));
                    return true;
                }
            }
        }
    } else {
        // Subdirectory
        uint32_t clusterSector = g_dataStart + (startCluster - 2) * g_bootSector.SectorsPerCluster;
        for (uint32_t s = 0; s < g_bootSector.SectorsPerCluster; s++) {
            if (!DISK_ReadSectors(g_disk, clusterSector + s, 1, sector))
                return false;
            FAT_DirectoryEntry* entries = (FAT_DirectoryEntry*)sector;
            for (uint32_t j = 0; j < entriesPerSector; j++) {
                if (entries[j].FileName[0] == 0x00) return false;
                if ((uint8_t)entries[j].FileName[0] == 0xE5) continue;
                if (match_filename(entries[j].FileName, fatname)) {
                    memcpy(outEntry, &entries[j], sizeof(FAT_DirectoryEntry));
                    return true;
                }
            }
        }
    }

    return false;
}

// -----------------------------------------------------------
// File Operations
// -----------------------------------------------------------

int i686_FAT_Open(const char* path) {
    if (strcmp(path, "/") == 0 || strcmp(path, "") == 0) {
        // Special: open root directory
        for (int fd = 0; fd < MAX_OPEN_FILES; fd++) {
            if (!g_openFiles[fd].open) {
                memset(&g_openFiles[fd], 0, sizeof(FAT_File));
                g_openFiles[fd].entry.LowFirstCluster = 0;
                g_openFiles[fd].entry.Attributes = ATTR_DIRECTORY;
                g_openFiles[fd].open = true;
                return fd;
            }
        }
        return -1; // No free slot (EMFILE)
    }

    char pathCopy[128];
    strncpy(pathCopy, path, sizeof(pathCopy));
    pathCopy[sizeof(pathCopy) - 1] = 0;

    char* token = strtok(pathCopy, "/");
    FAT_DirectoryEntry entry;
    uint32_t currentCluster = 0; // start in root

    while (token) {
        char fatname[11];
        to_fat_name(token, fatname);

        if (!FAT_FindInDirectory(currentCluster, fatname, &entry)) {
            return -2; // not found (ENOENT)
        }

        if (entry.Attributes & ATTR_DIRECTORY) {
            // move into subdir
            currentCluster = entry.LowFirstCluster;
            token = strtok(NULL, "/");
            continue;
        }

        // last component, must be file
        token = strtok(NULL, "/");
        if (token) return -3; // something after file → invalid (ENOTDIR)
    }

    // find free fd
    for (int fd = 0; fd < MAX_OPEN_FILES; fd++) {
        if (!g_openFiles[fd].open) {
            memcpy(&g_openFiles[fd].entry, &entry, sizeof(entry));
            g_openFiles[fd].currentCluster = entry.LowFirstCluster;
            g_openFiles[fd].currentOffset = 0;
            g_openFiles[fd].open = true;
            return fd;
        }
    }
    return -1;  // no free slot (EMFILE)
}

uint32_t i686_FAT_Read(int fd, void* buffer, uint32_t size) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;
    FAT_File* file = &g_openFiles[fd];
    if (!file->open) return -2;

    uint32_t clusterSector = g_dataStart + (file->currentCluster - 2) * g_bootSector.SectorsPerCluster;
    uint32_t toRead = (size > file->entry.Size ? file->entry.Size : size);

    if (!DISK_ReadSectors(g_disk, clusterSector, g_bootSector.SectorsPerCluster, buffer))
        return -3;

    file->currentOffset += toRead;
    return toRead;
}

void i686_FAT_Close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return;
    g_openFiles[fd].open = false;
}

bool i686_FAT_ReadEntry(int fd, uint32_t index, FAT_DirectoryEntry* outEntry) {
    FAT_File* file = i686_FAT_GetFileInfo(fd);
    if (!file) {
        return false; // Invalid fd
    }

    // Get base cluster or root directory
    uint32_t baseSector;
    uint32_t totalSectors;
    if (file->entry.LowFirstCluster == 0) {
        // Root directory
        baseSector = g_rootDirStart;
        totalSectors = g_rootDirSectors;
    } else {
        // Subdirectory cluster
        baseSector = g_dataStart + (file->entry.LowFirstCluster - 2) * g_bootSector.SectorsPerCluster;
        totalSectors = g_bootSector.SectorsPerCluster;
    }

    uint32_t entriesPerSector = g_bootSector.BytesPerSector / sizeof(FAT_DirectoryEntry);
    uint32_t sectorIndex = index / entriesPerSector;
    uint32_t entryIndex = index % entriesPerSector;

    if (sectorIndex >= totalSectors) {
        return false; // Index out of range
    }

    uint32_t targetSector = baseSector + sectorIndex;
    uint8_t sectorBuf[512];

    if (!DISK_ReadSectors(g_disk, targetSector, 1, sectorBuf)) {
        return false; // Failed to read sector, physically impossible according to DISK_ReadSectors definition but okay chat.
    }

    FAT_DirectoryEntry* entries = (FAT_DirectoryEntry*)sectorBuf;
    *outEntry = entries[entryIndex];

    // Skip deleted or empty entries
    if (outEntry->FileName[0] == 0x00 || (uint8_t)outEntry->FileName[0] == 0xE5)
        return false;

    return true;
}

uint32_t i686_FAT_Write(int fd, const void* buffer, uint32_t size) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return 0;
    FAT_File* file = &g_openFiles[fd];
    if (!file->open) return 0;

    uint32_t clusterSector = g_dataStart + (file->currentCluster - 2) * g_bootSector.SectorsPerCluster;
    uint32_t toWrite = size;

    // Write full sectors for the cluster(s)
    if (!DISK_WriteSectors(g_disk, clusterSector, g_bootSector.SectorsPerCluster, buffer))
        return 0;

    // Update file offset
    file->currentOffset += toWrite;

    // Optionally: update file size in directory entry (requires FAT update)
    if (file->currentOffset > file->entry.Size) {
        file->entry.Size = file->currentOffset;
        // TODO: write updated entry back to directory using DISK_WriteSectors
    }

    return toWrite;
}

// -----------------------------------------------------------
// Update an existing directory entry on disk
// -----------------------------------------------------------
bool FAT_UpdateDirectoryEntry(uint32_t startCluster, uint32_t entryIndex, FAT_DirectoryEntry* entry) {
    uint32_t baseSector;
    uint32_t totalSectors;
    uint32_t entriesPerSector = g_bootSector.BytesPerSector / sizeof(FAT_DirectoryEntry);

    if (startCluster == 0) {
        // Root directory
        baseSector = g_rootDirStart;
        totalSectors = g_rootDirSectors;
    } else {
        // Subdirectory
        baseSector = g_dataStart + (startCluster - 2) * g_bootSector.SectorsPerCluster;
        totalSectors = g_bootSector.SectorsPerCluster;
    }

    uint32_t sectorIndex = entryIndex / entriesPerSector;
    uint32_t entryOffset = entryIndex % entriesPerSector;

    if (sectorIndex >= totalSectors) return false;

    uint8_t sectorBuf[512];
    if (!DISK_ReadSectors(g_disk, baseSector + sectorIndex, 1, sectorBuf)) return false;

    FAT_DirectoryEntry* entries = (FAT_DirectoryEntry*)sectorBuf;
    entries[entryOffset] = *entry;

    if (!DISK_WriteSectors(g_disk, baseSector + sectorIndex, 1, sectorBuf)) return false;

    return true;
}

// -----------------------------------------------------------
// Create a new file in the root directory (or subdirectory)
// -----------------------------------------------------------
int i686_FAT_CreateFile(const char* path) {
    char pathCopy[128];
    strncpy(pathCopy, path, sizeof(pathCopy));
    pathCopy[sizeof(pathCopy) - 1] = 0;

    char* token = strtok(pathCopy, "/");
    FAT_DirectoryEntry entry;
    uint32_t currentCluster = 0; // start in root

    // Traverse directories to get parent folder
    char* lastToken = token;
    while (token) {
        lastToken = token;
        token = strtok(NULL, "/");
    }

    // lastToken is new file name
    char fatname[11];
    to_fat_name(lastToken, fatname);

    // Find a free directory entry in parent
    uint32_t entriesPerSector = g_bootSector.BytesPerSector / sizeof(FAT_DirectoryEntry);
    uint32_t totalSectors = (currentCluster == 0) ? g_rootDirSectors
        : g_bootSector.SectorsPerCluster;
    uint8_t sectorBuf[512];

    for (uint32_t s = 0; s < totalSectors; s++) {
        if (!DISK_ReadSectors(g_disk, g_rootDirStart + s, 1, sectorBuf)) continue;
        FAT_DirectoryEntry* entries = (FAT_DirectoryEntry*)sectorBuf;

        for (uint32_t i = 0; i < entriesPerSector; i++) {
            if (entries[i].FileName[0] == 0x00 || (uint8_t)entries[i].FileName[0] == 0xE5) {
                // Found empty slot
                memset(&entries[i], 0, sizeof(FAT_DirectoryEntry));
                memcpy(entries[i].FileName, fatname, 11);
                entries[i].Attributes = 0x00; // normal file
                entries[i].LowFirstCluster = 2; // TODO: allocate actual cluster
                entries[i].Size = 0;

                // Write sector back
                if (!DISK_WriteSectors(g_disk, g_rootDirStart + s, 1, sectorBuf))
                    return -1;

                // Open the file in memory
                for (int fd = 0; fd < MAX_OPEN_FILES; fd++) {
                    if (!g_openFiles[fd].open) {
                        memcpy(&g_openFiles[fd].entry, &entries[i], sizeof(FAT_DirectoryEntry));
                        g_openFiles[fd].currentCluster = entries[i].LowFirstCluster;
                        g_openFiles[fd].currentOffset = 0;
                        g_openFiles[fd].open = true;
                        return fd;
                    }
                }
                return -1;
            }
        }
    }

    return -1; // No free directory entry
}
