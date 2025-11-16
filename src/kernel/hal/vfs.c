#include "vfs.h"
#include <arch/i686/fat.h>
#include <arch/i686/key.h>
#include <arch/i686/disp.h>
#include <arch/i686/debug.h>

/* =========================
   WRITE
   ========================= */
size_t VFS_Write(int fd, const char* buffer, size_t amount) {
    if (!buffer || amount == 0) return 0;

    switch (fd) {

        case STDERR:
            i686_DEBUG_Debugs("\033[31m[ERR] ");
            for (size_t i = 0; i < amount; i++) {
                i686_DEBUG_Debugc(buffer[i]);
            }
            i686_DEBUG_Debugs("\033[0m\n");
            return amount;

        case STDIN:
        case STDOUT:
            for (size_t i = 0; i < amount; i++) {
                i686_DISP_PutChar(buffer[i]);
            }
            return amount;

        default:
            return i686_FAT_Write(fd - 3, buffer, amount);
    }
}

/* =========================
   READ
   ========================= */
size_t VFS_Read(int fd, char* buffer, size_t amount) {
    if (!buffer || amount == 0) return 0;

    switch (fd) {

        case STDERR:
            return 0;

        case STDOUT:
            return 0;

        case STDIN: {
            size_t i = 0;
            while (i < amount) {
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

        default:
            return i686_FAT_Read(fd - 3, buffer, amount);
    }
}

int VFS_Open(const char* path) {
    int fd = i686_FAT_Open(path);
    if (fd < 0) return -1;
    return fd + 3;   // shift by reserved FDs
}

void VFS_Close(int fd) {
    if (fd >= 3) {
        i686_FAT_Close(fd - 3);
    }
}
