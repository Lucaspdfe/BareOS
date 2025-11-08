#include "pic.h"
#include "io.h"

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

void i686_PIC_SendCommand(uint8_t pic, uint8_t value) {
    if (pic > 2 || pic < 1) return;
    if (pic == 1) {
        i686_outb(PIC1_COMMAND_PORT, value);
    }
    else {
        i686_outb(PIC2_COMMAND_PORT, value);
    }
    i686_IOWait();
}

void i686_PIC_SendData(uint8_t pic, uint8_t value) {
    if (pic > 2 || pic < 1) return;
    if (pic == 1) {
        i686_outb(PIC1_DATA_PORT, value);
    }
    else {
        i686_outb(PIC2_DATA_PORT, value);
    }
    i686_IOWait();
}

uint8_t i686_PIC_ReadData(uint8_t pic) {
    uint8_t data;
    if (pic > 2 || pic < 1) return 0;
    if (pic == 1) {
        data = i686_inb(PIC1_DATA_PORT);
    }
    else {
        data = i686_inb(PIC2_DATA_PORT);
    }
    i686_IOWait();
    return data;
}

uint8_t i686_PIC_ReadCommand(uint8_t pic) {
    uint8_t command;
    if (pic > 2 || pic < 1) return 0;
    if (pic == 1) {
        command = i686_inb(PIC1_COMMAND_PORT);
    }
    else {
        command = i686_inb(PIC2_COMMAND_PORT);
    }
    i686_IOWait();
    return command;
}

void i686_PIC_Configure(uint8_t offsetPic1, uint8_t offsetPic2) {
    // Start Initialization
    i686_PIC_SendCommand(1, PIC_ICW1_INITIALIZE | PIC_ICW1_ICW4);
    i686_PIC_SendCommand(2, PIC_ICW1_INITIALIZE | PIC_ICW1_ICW4);
    // ICW2: PIC vector offset
    i686_PIC_SendData(1, offsetPic1);
    i686_PIC_SendData(2, offsetPic2);
    // ICW3: Cascade Configuration
    i686_PIC_SendData(1, 1 << 2);
    i686_PIC_SendData(2, 2);
    // ICW4: PICs use 8086 mode (default 8080 mode)
    i686_PIC_SendData(1, PIC_ICW4_8086);
    i686_PIC_SendData(2, PIC_ICW4_8086);


    // Unmask both PICs
    i686_PIC_SendData(1, 0);
    i686_PIC_SendData(2, 0);
}

void i686_PIC_SendEndOfInterrupt(int irq) {
    if(irq >= 8)
		i686_outb(PIC2_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
	
	i686_outb(PIC1_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
}

void i686_PIC_Disable() {
    i686_PIC_SendData(1, 0xFF);
    i686_PIC_SendData(2, 0xFF);
}

void i686_PIC_Mask(int irq) {
    uint8_t pic;
    uint8_t value;

    if(irq < 8) {
        pic = 1;
    } else {
        pic = 2;
        irq -= 8;
    }
    value = i686_PIC_ReadData(pic) | (1 << irq);
    i686_PIC_SendData(pic, value);
}

void i686_PIC_Unmask(int irq) {
    uint8_t pic;
    uint8_t value;

    if(irq < 8) {
        pic = 1;
    } else {
        pic = 2;
        irq -= 8;
    }
    value = i686_PIC_ReadData(pic) & ~(1 << irq);
    i686_PIC_SendData(pic, value);
}

uint16_t i686_PIC_ReadIrqRequestRegister() {
    i686_PIC_SendCommand(1, PIC_CMD_READ_IRR);
    i686_PIC_SendCommand(2, PIC_CMD_READ_IRR);
    return (i686_PIC_ReadData(2) << 8) | i686_PIC_ReadData(1);
}

uint16_t i686_PIC_ReadInServiceRegister() {
    i686_PIC_SendCommand(1, PIC_CMD_READ_ISR);
    i686_PIC_SendCommand(2, PIC_CMD_READ_ISR);
    return (i686_PIC_ReadData(2) << 8) | i686_PIC_ReadData(1);
}