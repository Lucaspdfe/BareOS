#include <stdint.h>
#include <stddef.h>
#include "stdio.h"
#include <arch/i686/disp.h>
#include <hal/vfs.h>

/* Basic stdio layer built on top of the i686 DISP driver. */

void putc(char c) {
    /* pass pointer to the single char */
    VFS_Write(STDOUT, &c, 1);
}

void puts(const char* s) {
    /* compute string length and write, then append newline (like POSIX puts) (This isn't POSIX so i do whatever the hell i want, so no append newline) */
    size_t len = 0;
    while (s && s[len]) ++len;
    if (len > 0) VFS_Write(STDOUT, s, len);
}

void clrscr() {
    i686_DISP_Clear();
}