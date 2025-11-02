#include <stdint.h>
#include <stddef.h>
#include "stdio.h"
#include <arch/i686/disp.h>

/* Basic stdio layer built on top of the i686 DISP driver. */

void putc(char c) {
    i686_DISP_PutChar(c);
}

void puts(const char* s) {
    i686_DISP_PutString(s);
}
