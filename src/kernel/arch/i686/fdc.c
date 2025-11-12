#include "fdc.h"
#include "io.h"
#include "irq.h"
#include <string.h>
#include <stdint.h>

#define FDC_DMA_BUFFER_ADDR ((void*)0x30000)
#define FDC_SECTOR_SIZE     512

#define FDC_DOR   0x3F2
#define FDC_MSR   0x3F4
#define FDC_DATA  0x3F5

#define MSR_RQM 0x80
#define MSR_DIO 0x40

volatile int i686_FDC_DmaCompleteFlag = 0;

static void i686_FDC_WaitReady(int forRead) {
    uint8_t msr;
    do {
        msr = i686_inb(FDC_MSR);
    } while (!(msr & MSR_RQM) || ((msr & MSR_DIO) != (forRead ? MSR_DIO : 0)));
}

static void i686_FDC_SendCommand(uint8_t cmd) {
    i686_FDC_WaitReady(0);
    i686_outb(FDC_DATA, cmd);
}

static uint8_t i686_FDC_ReceiveData() {
    i686_FDC_WaitReady(1);
    return i686_inb(FDC_DATA);
}

static void i686_FDC_IRQ_Handler(Registers* regs) {
    i686_FDC_DmaCompleteFlag = 1;
}

static void i686_FDC_DmaSetup(uint32_t addr, uint16_t length, int read) {
    i686_outb(0x0A, 0x06);      // mask channel 2
    i686_outb(0x0C, 0x00);      // clear flip-flop
    i686_outb(0x04, addr & 0xFF);
    i686_outb(0x04, (addr >> 8) & 0xFF);
    i686_outb(0x81, (addr >> 16) & 0xFF);
    i686_outb(0x05, (length-1) & 0xFF);
    i686_outb(0x05, ((length-1) >> 8) & 0xFF);
    i686_outb(0x0B, read ? 0x46 : 0x4A);
    i686_outb(0x0A, 0x02);      // unmask channel 2
}

static void i686_FDC_EnableDrive0Motor() {
    i686_outb(FDC_DOR, 0x1C);
    i686_IOWait();
}

static void i686_FDC_DisableMotor() {
    i686_outb(FDC_DOR, 0x0C);
    i686_IOWait();
}

void i686_FDC_Reset() {
    i686_outb(FDC_DOR, 0x00);
    i686_IOWait();
    i686_outb(FDC_DOR, 0x1C);
    i686_IOWait();
}

void i686_FDC_Initialize() {
    i686_IRQ_RegisterHandler(6, i686_FDC_IRQ_Handler);
    i686_FDC_Reset();

    i686_FDC_SendCommand(0x10);
    uint8_t version = i686_FDC_ReceiveData();
    if (version != 0x90) i686_Panic();

    i686_FDC_SendCommand(0x13);
    i686_FDC_SendCommand(0x00);
    i686_FDC_SendCommand(0xD0);
    i686_FDC_SendCommand(0x00);
}

//----------------------------------------
// Fixed single-sector read
//----------------------------------------
void i686_FDC_ReadSector(uint8_t track, uint8_t head, uint8_t sector, void* buffer) {
    uint8_t result[7];

    i686_FDC_EnableDrive0Motor();
    i686_FDC_DmaCompleteFlag = 0;
    i686_FDC_DmaSetup((uint32_t)FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE, 1);

    i686_FDC_SendCommand(0x66);           // READ DATA
    i686_FDC_SendCommand((head << 2) | 0);
    i686_FDC_SendCommand(track);
    i686_FDC_SendCommand(head);
    i686_FDC_SendCommand(sector);
    i686_FDC_SendCommand(0x02);           // 512 bytes
    i686_FDC_SendCommand(0x01);           // sector count
    i686_FDC_SendCommand(0x1B);           // GAP3
    i686_FDC_SendCommand(0xFF);           // data length

    while (!i686_FDC_DmaCompleteFlag);    // wait for IRQ

    // Read Result Phase (important to clear controller)
    for (int i=0; i<7; i++)
        result[i] = i686_FDC_ReceiveData();

    memcpy(buffer, FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE);
    i686_FDC_DisableMotor();
}

//----------------------------------------
// Fixed multi-sector read
//----------------------------------------
void i686_FDC_ReadSectors(uint8_t track, uint8_t head, uint8_t startSector, uint8_t count, void* buffer) {
    uint8_t result[7];

    if (count == 0 || count > 18) return; // safety check for standard 1.44MB floppy

    i686_FDC_EnableDrive0Motor();

    i686_FDC_DmaCompleteFlag = 0;
    i686_FDC_DmaSetup((uint32_t)FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE * count, 1);

    // READ DATA (MFM, single track)
    i686_FDC_SendCommand(0x46);
    i686_FDC_SendCommand((head << 2) | 0);      // Head & drive
    i686_FDC_SendCommand(track);                // Track number
    i686_FDC_SendCommand(head);                 // Head
    i686_FDC_SendCommand(startSector);          // Starting sector
    i686_FDC_SendCommand(0x02);                 // Sector size (512 bytes)
    i686_FDC_SendCommand(startSector + count - 1); // Last sector to read (EOT)
    i686_FDC_SendCommand(0x1B);                 // GAP3
    i686_FDC_SendCommand(0xFF);                 // Data length

    // Wait for IRQ
    while (!i686_FDC_DmaCompleteFlag);

    // Result phase: read 7 status bytes
    for (int i = 0; i < 7; i++)
        result[i] = i686_FDC_ReceiveData();

    // Copy from DMA buffer to destination
    memcpy(buffer, FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE * count);

    i686_FDC_DisableMotor();
}

//----------------------------------------
// Fixed single-sector write
//----------------------------------------
void i686_FDC_WriteSector(uint8_t track, uint8_t head, uint8_t sector, const void* buffer) {
    uint8_t result[7];

    i686_FDC_EnableDrive0Motor();
    memcpy(FDC_DMA_BUFFER_ADDR, buffer, FDC_SECTOR_SIZE);
    i686_FDC_DmaCompleteFlag = 0;
    i686_FDC_DmaSetup((uint32_t)FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE, 0);

    i686_FDC_SendCommand(0xC5);           // WRITE DATA
    i686_FDC_SendCommand((head << 2) | 0);
    i686_FDC_SendCommand(track);
    i686_FDC_SendCommand(head);
    i686_FDC_SendCommand(sector);
    i686_FDC_SendCommand(0x02);
    i686_FDC_SendCommand(0x01);
    i686_FDC_SendCommand(0x1B);
    i686_FDC_SendCommand(0xFF);

    while (!i686_FDC_DmaCompleteFlag);

    for (int i=0; i<7; i++)
        result[i] = i686_FDC_ReceiveData();

    i686_FDC_DisableMotor();
}

//----------------------------------------
// Fixed multi-sector write
//----------------------------------------
void i686_FDC_WriteSectors(uint8_t track, uint8_t head, uint8_t startSector, uint8_t count, const void* buffer) {
    uint8_t result[7];

    if (count == 0 || count > 18) return; // safety check (1.44MB floppy max sectors per track)

    i686_FDC_EnableDrive0Motor();

    // Copy data to DMA buffer
    memcpy(FDC_DMA_BUFFER_ADDR, buffer, FDC_SECTOR_SIZE * count);

    // Setup DMA for write (read=0)
    i686_FDC_DmaCompleteFlag = 0;
    i686_FDC_DmaSetup((uint32_t)FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE * count, 0);

    // WRITE DATA (MFM, multi-sector)
    i686_FDC_SendCommand(0x45);                 // 0x45 = WRITE DATA, MFM
    i686_FDC_SendCommand((head << 2) | 0);      // Head + drive
    i686_FDC_SendCommand(track);                // Cylinder
    i686_FDC_SendCommand(head);                 // Head
    i686_FDC_SendCommand(startSector);          // Starting sector
    i686_FDC_SendCommand(0x02);                 // 512 bytes per sector (2 = 512)
    i686_FDC_SendCommand(startSector + count - 1); // End sector (EOT)
    i686_FDC_SendCommand(0x1B);                 // GAP3 length
    i686_FDC_SendCommand(0xFF);                 // Data length (unused)

    // Wait for IRQ (DMA complete)
    while (!i686_FDC_DmaCompleteFlag);

    // Read status bytes (Result Phase)
    for (int i = 0; i < 7; i++)
        result[i] = i686_FDC_ReceiveData();

    i686_FDC_DisableMotor();
}
