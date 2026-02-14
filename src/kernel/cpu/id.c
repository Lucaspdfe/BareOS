#include "id.h"
#include <stdint.h>
#include <string.h>

bool ID_APICSupport() {
    if (!ID_CheckSupported()) return false;

    uint32_t eax, ebx, ecx, edx;

    __asm volatile(
        "mov $1, %%eax\n"
        "cpuid\n"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        :
        :
    );

    return (edx & (1 << 9)) != 0;
}

bool ID_CPUName(char* brand) {
    if (!ID_CheckSupported()) return false;

    uint32_t regs[4];

    for (uint32_t i = 0; i < 3; i++) {
        __asm volatile (
            "cpuid"
            : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
            : "a"(0x80000002 + i)
        );
        memcpy(brand + i * 16, regs, 16);
    }
    return true;
}
