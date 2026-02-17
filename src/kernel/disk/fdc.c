#include "fdc.h"
#include <io/io.h>
#include <cpu/irq.h>
#include <cpu/cmos.h>
#include <string.h>
#include <stdint.h>

#define FDC_DMA_BUFFER_ADDR ((void*)0x30000)
#define FDC_SECTOR_SIZE     512

#define FDC_DOR   0x3F2
#define FDC_MSR   0x3F4
#define FDC_DATA  0x3F5

#define MSR_RQM 0x80
#define MSR_DIO 0x40

volatile int g_DmaCompleteFlag = 0;

static void FDC_WaitReady(int forRead) {
    uint8_t msr;
    do {
        msr = inb(FDC_MSR);
    } while (!(msr & MSR_RQM) || ((msr & MSR_DIO) != (forRead ? MSR_DIO : 0)));
}

static void FDC_SendCommand(uint8_t cmd) {
    FDC_WaitReady(0);
    outb(FDC_DATA, cmd);
}

static uint8_t FDC_ReceiveData() {
    FDC_WaitReady(1);
    return inb(FDC_DATA);
}

static void FDC_IRQ_Handler(Registers* regs) {
    g_DmaCompleteFlag = 1;
}

static void FDC_DmaSetup(uint32_t addr, uint16_t length, int read) {
    outb(0x0A, 0x06);      // mask channel 2
    outb(0x0C, 0x00);      // clear flip-flop
    outb(0x04, addr & 0xFF);
    outb(0x04, (addr >> 8) & 0xFF);
    outb(0x81, (addr >> 16) & 0xFF);
    outb(0x05, (length-1) & 0xFF);
    outb(0x05, ((length-1) >> 8) & 0xFF);
    outb(0x0B, read ? 0x46 : 0x4A);
    outb(0x0A, 0x02);      // unmask channel 2
}

static void FDC_EnableMotor(uint8_t drive) {
    uint8_t dor = 0x0C;                  // controller + IRQ/DMA enabled
    dor |= (1 << drive);                 // select drive (bits 0–1)
    dor |= (1 << (4 + drive));           // enable motor
    outb(FDC_DOR, dor);
    IOWait();
}

static void FDC_DisableMotor(uint8_t drive) {
    uint8_t dor = 0x0C;                  // controller + IRQ/DMA enabled
    dor |= (1 << drive);                 // select drive
    /* motor bit intentionally not set */
    outb(FDC_DOR, dor);
    IOWait();
}

void FDC_Reset() {
    outb(FDC_DOR, 0x00);
    IOWait();
    outb(FDC_DOR, 0x1C);
    IOWait();
}

void FDC_Initialize(uint8_t drive) {
    IRQ_RegisterHandler(6, FDC_IRQ_Handler);

    FDC_Reset();

    /* Enable motor for selected drive */
    FDC_EnableMotor(drive);

    /* Select drive (do NOT clear motor bit) */
    outb(FDC_DOR, 0x0C | drive | (1 << (4 + drive)));
    IOWait();

    /* Motor spin-up delay (~500ms real HW) */
    for (volatile int i = 0; i < 1000000; i++);

    /* ---- VERSION (no IRQ) ---- */
    FDC_SendCommand(0x10);
    uint8_t version = FDC_ReceiveData();
    if (version != 0x90) Panic();

    /* ---- RECALIBRATE (IRQ) ---- */
    g_DmaCompleteFlag = 0;        // arm IRQ
    FDC_SendCommand(0x07);
    FDC_SendCommand(drive);
    while (!g_DmaCompleteFlag);

    /* Clear interrupt status */
    FDC_SendCommand(0x08);       // SENSE INTERRUPT STATUS
    FDC_ReceiveData();           // ST0
    FDC_ReceiveData();           // Cylinder

    /* ---- CONFIGURE (no IRQ) ---- */
    FDC_SendCommand(0x13);
    FDC_SendCommand(0x00);
    FDC_SendCommand(0xD0);
    FDC_SendCommand(0x00);

    /* ---- SPECIFY (no IRQ) ---- */
    FDC_SendCommand(0x03);
    FDC_SendCommand(0xDF);
    FDC_SendCommand(0x02);
}

//----------------------------------------
// Fixed single-sector read
//----------------------------------------
void FDC_ReadSector(uint8_t drive, uint8_t track, uint8_t head, uint8_t sector, void* buffer) {
    uint8_t result[7];

    FDC_EnableMotor(drive);
    g_DmaCompleteFlag = 0;
    FDC_DmaSetup((uint32_t)FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE, 1);

    FDC_SendCommand(0x66);                // READ DATA
    FDC_SendCommand((head << 2) | drive); // Head & drive
    FDC_SendCommand(track);
    FDC_SendCommand(head);
    FDC_SendCommand(sector);
    FDC_SendCommand(0x02);                // 512 bytes
    FDC_SendCommand(0x01);                // sector count
    FDC_SendCommand(0x1B);                // GAP3
    FDC_SendCommand(0xFF);                // data length

    while (!g_DmaCompleteFlag);    // wait for IRQ

    // Read Result Phase (important to clear controller)
    for (int i=0; i<7; i++)
        result[i] = FDC_ReceiveData();

    memcpy(buffer, FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE);
    FDC_DisableMotor(drive);
}

//----------------------------------------
// Fixed multi-sector read
//----------------------------------------
void FDC_ReadSectors(uint8_t drive, uint8_t track, uint8_t head, uint8_t startSector, uint8_t count, void* buffer) {
    uint8_t result[7];

    if (count == 0 || count > 18) return; // safety check for standard 1.44MB floppy

    FDC_EnableMotor(drive);

    g_DmaCompleteFlag = 0;
    FDC_DmaSetup((uint32_t)FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE * count, 1);

    // READ DATA (MFM, single track)
    FDC_SendCommand(0x46);
    FDC_SendCommand((head << 2) | drive);  // Head & drive
    FDC_SendCommand(track);                // Track number
    FDC_SendCommand(head);                 // Head
    FDC_SendCommand(startSector);          // Starting sector
    FDC_SendCommand(0x02);                 // Sector size (512 bytes)
    FDC_SendCommand(startSector + count - 1); // Last sector to read (EOT)
    FDC_SendCommand(0x1B);                 // GAP3
    FDC_SendCommand(0xFF);                 // Data length

    // Wait for IRQ
    while (!g_DmaCompleteFlag);

    // Result phase: read 7 status bytes
    for (int i = 0; i < 7; i++)
        result[i] = FDC_ReceiveData();

    // Copy from DMA buffer to destination
    memcpy(buffer, FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE * count);

    FDC_DisableMotor(drive);
}

//----------------------------------------
// Fixed single-sector write
//----------------------------------------
void FDC_WriteSector(uint8_t drive, uint8_t track, uint8_t head, uint8_t sector, const void* buffer) {
    uint8_t result[7];

    FDC_EnableMotor(drive);
    memcpy(FDC_DMA_BUFFER_ADDR, buffer, FDC_SECTOR_SIZE);
    g_DmaCompleteFlag = 0;
    FDC_DmaSetup((uint32_t)FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE, 0);

    FDC_SendCommand(0xC5);                // WRITE DATA
    FDC_SendCommand((head << 2) | drive); // Head & drive
    FDC_SendCommand(track);
    FDC_SendCommand(head);
    FDC_SendCommand(sector);
    FDC_SendCommand(0x02);
    FDC_SendCommand(0x01);
    FDC_SendCommand(0x1B);
    FDC_SendCommand(0xFF);

    while (!g_DmaCompleteFlag);

    for (int i=0; i<7; i++)
        result[i] = FDC_ReceiveData();

    FDC_DisableMotor(drive);
}

//----------------------------------------
// Fixed multi-sector write
//----------------------------------------
void FDC_WriteSectors(uint8_t drive, uint8_t track, uint8_t head, uint8_t startSector, uint8_t count, const void* buffer) {
    uint8_t result[7];

    if (count == 0 || count > 18) return; // safety check (1.44MB floppy max sectors per track)

    FDC_EnableMotor(drive);

    // Copy data to DMA buffer
    memcpy(FDC_DMA_BUFFER_ADDR, buffer, FDC_SECTOR_SIZE * count);

    // Setup DMA for write (read=0)
    g_DmaCompleteFlag = 0;
    FDC_DmaSetup((uint32_t)FDC_DMA_BUFFER_ADDR, FDC_SECTOR_SIZE * count, 0);

    // WRITE DATA (MFM, multi-sector)
    FDC_SendCommand(0x45);                 // 0x45 = WRITE DATA, MFM
    FDC_SendCommand((head << 2) | drive);  // Head + drive
    FDC_SendCommand(track);                // Cylinder
    FDC_SendCommand(head);                 // Head
    FDC_SendCommand(startSector);          // Starting sector
    FDC_SendCommand(0x02);                 // 512 bytes per sector (2 = 512)
    FDC_SendCommand(startSector + count - 1); // End sector (EOT)
    FDC_SendCommand(0x1B);                 // GAP3 length
    FDC_SendCommand(0xFF);                 // Data length (unused)

    // Wait for IRQ (DMA complete)
    while (!g_DmaCompleteFlag);

    // Read status bytes (Result Phase)
    for (int i = 0; i < 7; i++)
        result[i] = FDC_ReceiveData();

    FDC_DisableMotor(drive);
}

void FDC_GetInfo(uint8_t drive, uint8_t* maxCylinders, uint8_t* maxHeads, uint8_t* maxSectors) {
    uint8_t drives[2];
    CMOS_GetFloppyType(&drives[0], &drives[1]);

    if (drive < 2 && drives[drive] != 0) {
        // 0 = None, 1 = 360KB, 2 = 1.2MB, 3 = 720KB, 4 = 1.44MB, 5 = 2.88MB
        if (drives[drive] == 0x05) {                    // 2.88MB
            *maxCylinders = 80;
            *maxHeads = 2;
            *maxSectors = 36;
        } else if (drives[drive] == 0x04) {             // 1.44MB
            *maxCylinders = 80;
            *maxHeads = 2;
            *maxSectors = 18;
        } else if (drives[drive] == 0x03) {             // 720KB
            *maxCylinders = 80;
            *maxHeads = 2;
            *maxSectors = 9;
        } else if (drives[drive] == 0x02) {             // 1.2MB
            *maxCylinders = 80;
            *maxHeads = 2;
            *maxSectors = 15;
        } else if (drives[drive] == 0x01) {             // 360KB
            *maxCylinders = 40;
            *maxHeads = 2;
            *maxSectors = 9;
        } else {                                        // None?
            // Unknown type, default to no drive
            *maxCylinders = 0;
            *maxHeads = 0;
            *maxSectors = 0;
        }
    } else {
        *maxCylinders = 0;
        *maxHeads = 0;
        *maxSectors = 0;
    }
}
