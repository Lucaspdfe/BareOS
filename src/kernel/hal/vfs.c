#include "vfs.h"
#include <arch/i686/key.h>
#include <arch/i686/disp.h>

/* POSIX-like VFS write: return number of bytes written or -1 on error */
size_t VFS_Write(int fd, const char* buffer, size_t amount) {
    if (!buffer || amount == 0) return 0;
    switch (fd) {
        case STDERR:
        case STDOUT:
            for (size_t i = 0; i < amount; i++) {
                i686_DISP_PutChar(buffer[i]);
            }
            return (size_t)amount;
        default:
            return -1;
    }
}

/* POSIX-like VFS read: reads up to `amount` bytes into buffer; returns bytes read or -1 on error.
   Behavior: for STDIN, blocks until characters arrive. Echoes characters to display and handles backspace.
*/
size_t VFS_Read(int fd, char* buffer, size_t amount) {
    if (!buffer || amount == 0) return 0;
    if (fd != STDIN) return -1;

    size_t i = 0;
    while (i < amount) {
        KEYState state = i686_KEY_WaitKey();
        if (state.isSpecial) {
            /* Handle Caps and ignore arrow keys here */
            if (state.specialKey == KEY_CAPS) continue;
            if (state.specialKey == KEY_ARROW_LEFT || state.specialKey == KEY_ARROW_RIGHT ||
                state.specialKey == KEY_ARROW_UP || state.specialKey == KEY_ARROW_DOWN) {
                continue;
            }
            continue;
        }

        char c = state.character;
        if (c == '\0') continue; /* ignore empty */

        if (c == '\b') {
            if (i > 0) {
                i--;
                /* echo backspace which will move cursor and clear the glyph */
                i686_DISP_PutChar('\b');
            }
            continue;
        }

        /* store and echo */
        buffer[i++] = c;
        i686_DISP_PutChar(c);
        if (c == '\n') break; /* stop at newline (line-oriented) */
    }
    return (size_t)i;
}