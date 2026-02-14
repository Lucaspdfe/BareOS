#include "pic.h"
#include <io/io.h>

#define PIC1_COMMAND_PORT           0x20
#define PIC1_DATA_PORT              0x21
#define PIC2_COMMAND_PORT           0xA0
#define PIC2_DATA_PORT              0xA1

// Initialization Control Word 1
// -----------------------------
//  0   IC4     if set, the PIC expects to receive ICW4 during initialization
//  1   SGNL    if set, only 1 PIC in the system; if unset, the PIC is cascaded with slave PICs
//              and ICW3 must be sent to controller
//  2   ADI     call address interval, set: 4, not set: 8; ignored on x86, set to 0
//  3   LTIM    if set, operate in level triggered mode; if unset, operate in edge triggered mode
//  4   INIT    set to 1 to initialize PIC
//  5-7         ignored on x86, set to 0

enum PIC_ICW1 {
    PIC_ICW1_ICW4           = 0x01,
    PIC_ICW1_SINGLE         = 0x02,
    PIC_ICW1_INTERVAL4      = 0x04,
    PIC_ICW1_LEVEL          = 0x08,
    PIC_ICW1_INITIALIZE     = 0x10
};


// Initialization Control Word 4
// -----------------------------
//  0   uPM     if set, PIC is in 80x86 mode; if cleared, in MCS-80/85 mode
//  1   AEOI    if set, on last interrupt acknowledge pulse, controller automatically performs 
//              end of interrupt operation
//  2   M/S     only use if BUF is set; if set, selects buffer master; otherwise, selects buffer slave
//  3   BUF     if set, controller operates in buffered mode
//  4   SFNM    specially fully nested mode; used in systems with large number of cascaded controllers
//  5-7         reserved, set to 0
enum PIC_ICW4 {
    PIC_ICW4_8086           = 0x1,
    PIC_ICW4_AUTO_EOI       = 0x2,
    PIC_ICW4_BUFFER_MASTER  = 0x4,
    PIC_ICW4_BUFFER_SLAVE   = 0x0,
    PIC_ICW4_BUFFERRED      = 0x8,
    PIC_ICW4_SFNM           = 0x10,
};


enum PIC_CMD {
    PIC_CMD_END_OF_INTERRUPT    = 0x20,
    PIC_CMD_READ_IRR            = 0x0A,
    PIC_CMD_READ_ISR            = 0x0B,
};

void PIC_SendCommand(uint8_t pic, uint8_t value) {
    if (pic > 2 || pic < 1) return;
    if (pic == 1) {
        outb(PIC1_COMMAND_PORT, value);
    }
    else {
        outb(PIC2_COMMAND_PORT, value);
    }
    IOWait();
}

void PIC_SendData(uint8_t pic, uint8_t value) {
    if (pic > 2 || pic < 1) return;
    if (pic == 1) {
        outb(PIC1_DATA_PORT, value);
    }
    else {
        outb(PIC2_DATA_PORT, value);
    }
    IOWait();
}

uint8_t PIC_ReadData(uint8_t pic) {
    uint8_t data;
    if (pic > 2 || pic < 1) return 0;
    if (pic == 1) {
        data = inb(PIC1_DATA_PORT);
    }
    else {
        data = inb(PIC2_DATA_PORT);
    }
    IOWait();
    return data;
}

uint8_t PIC_ReadCommand(uint8_t pic) {
    uint8_t command;
    if (pic > 2 || pic < 1) return 0;
    if (pic == 1) {
        command = inb(PIC1_COMMAND_PORT);
    }
    else {
        command = inb(PIC2_COMMAND_PORT);
    }
    IOWait();
    return command;
}

void PIC_Configure(uint8_t offsetPic1, uint8_t offsetPic2) {
    // Start Initialization
    PIC_SendCommand(1, PIC_ICW1_INITIALIZE | PIC_ICW1_ICW4);
    PIC_SendCommand(2, PIC_ICW1_INITIALIZE | PIC_ICW1_ICW4);
    // ICW2: PIC vector offset
    PIC_SendData(1, offsetPic1);
    PIC_SendData(2, offsetPic2);
    // ICW3: Cascade Configuration
    PIC_SendData(1, 1 << 2);
    PIC_SendData(2, 2);
    // ICW4: PICs use 8086 mode (default 8080 mode)
    PIC_SendData(1, PIC_ICW4_8086);
    PIC_SendData(2, PIC_ICW4_8086);

    // Unmask both PICs
    PIC_SendData(1, 0);
    PIC_SendData(2, 0);
}

void PIC_SendEndOfInterrupt(int irq) {
    if(irq >= 8)
		outb(PIC2_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
	
	outb(PIC1_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
}

void PIC_Disable() {
    PIC_SendData(1, 0xFF);
    PIC_SendData(2, 0xFF);
}

void PIC_Mask(int irq) {
    uint8_t pic;
    uint8_t value;

    if(irq < 8) {
        pic = 1;
    } else {
        pic = 2;
        irq -= 8;
    }
    value = PIC_ReadData(pic) | (1 << irq);
    PIC_SendData(pic, value);
}

void PIC_Unmask(int irq) {
    uint8_t pic;
    uint8_t value;

    if(irq < 8) {
        pic = 1;
    } else {
        pic = 2;
        irq -= 8;
    }
    value = PIC_ReadData(pic) & ~(1 << irq);
    PIC_SendData(pic, value);
}

uint16_t PIC_ReadIrqRequestRegister() {
    PIC_SendCommand(1, PIC_CMD_READ_IRR);
    PIC_SendCommand(2, PIC_CMD_READ_IRR);
    return (PIC_ReadData(2) << 8) | PIC_ReadData(1);
}

uint16_t PIC_ReadInServiceRegister() {
    PIC_SendCommand(1, PIC_CMD_READ_ISR);
    PIC_SendCommand(2, PIC_CMD_READ_ISR);
    return (PIC_ReadData(2) << 8) | PIC_ReadData(1);
}