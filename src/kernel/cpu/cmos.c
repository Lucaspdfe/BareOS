#include "cmos.h"
#include <io/io.h>
#include <stdint.h>

uint8_t CMOS_Read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

void CMOS_Wait() {
    while (CMOS_Read(0x0A) & 0x80);
}

uint32_t CMOS_BCD2BIN(uint32_t val) {
    return (val & 0x0F) + ((val / 16) * 10);
}

void CMOS_RTCRead(uint32_t* sec, uint32_t* min, uint32_t* hour, uint32_t* day, uint32_t* month, uint32_t* year) {
    uint8_t regB = CMOS_Read(0x0B);

    CMOS_Wait();

    *sec    = CMOS_Read(0x00);
    *min    = CMOS_Read(0x02);
    *hour   = CMOS_Read(0x04);
    *day    = CMOS_Read(0x07);
    *month  = CMOS_Read(0x08);
    *year   = CMOS_Read(0x09);

    if (!(regB & 0x04)) {
        *sec    = CMOS_BCD2BIN(*sec);
        *min    = CMOS_BCD2BIN(*min);
        *hour   = CMOS_BCD2BIN(*hour);
        *day    = CMOS_BCD2BIN(*day);
        *month  = CMOS_BCD2BIN(*month);
        *year   = CMOS_BCD2BIN(*year);
    }

    if (*year < 70) {
        *year += 2000;
    } else if (*year >= 70) {
        *year += 1900;
    }
}