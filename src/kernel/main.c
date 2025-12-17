#include <stdint.h>
#include <stdio.h>
#include <hal/hal.h>
#include <hal/vfs.h>
#include <arch/i686/key.h>
#include <arch/i686/disp.h>
#include "exec.h"

#define PROGRAM_LOAD_ADDR  ((uint32_t)0x00200000)
#define PROGRAM_STACK_TOP  ((uint32_t)0x0020F000)

void __attribute__((section(".entry"))) start(void* tags) {
    HAL_Initialize(tags);

    // Minimal kernel loader-shell: prompt for program path and run it
    i686_DISP_PutString("BareOS loader\n");

    char path[128];
    for (;;) {
        i686_DISP_PutString("run> ");

        int pos = 0;
        while (1) {
            KEYState st = i686_KEY_WaitKey();
            if (st.isSpecial) continue;
            char c = st.character;
            if (c == '\r' || c == '\n') {
                i686_DISP_PutChar('\n');
                break;
            }
            if (c == '\b') {
                if (pos > 0) {
                    pos--;
                    i686_DISP_PutChar('\b');
                }
                continue;
            }
            if (pos < (int)sizeof(path) - 1) {
                path[pos++] = c;
                i686_DISP_PutChar(c);
            }
        }
        path[pos] = '\0';

        if (pos == 0) continue;

        // Tokenize path into argv (split by spaces)
        char* argv[16];
        int argc = 0;
        char* p = path;
        while (*p && argc < 16) {
            while (*p == ' ') p++;
            if (!*p) break;
            argv[argc++] = p;
            while (*p && *p != ' ') p++;
            if (*p == ' ') { *p = '\0'; p++; }
        }

        size_t size = LoadProgram(path);
        if (size == 0) {
            i686_DISP_PutString("load failed\n");
            continue;
        }

        // prepare user stack with argv
        uint32_t user_sp = PrepareUserStack(PROGRAM_STACK_TOP, argv, argc);
        JumpToProgram(PROGRAM_LOAD_ADDR, user_sp);
        /* If the program returns, fall back to the prompt */
    }
}
