# Memory Management

This is the place where i put every address in memory.

0x00000000 --> 0x000003FF - BIOS IVT                    -|
0x00000400 --> 0x000004FF - BIOS Data                    |
0x00000500 --> 0x0002FFFF - Free                         |
0x00030000 --> 0x000301FF - Floppy Disk driver           |
0x00030200 --> 0x0007FFFF - Free                         |
0x00080000 --> 0x0009FFFF - Extended BIOS data area      |- PageTable0
0x000A0000 --> 0x000C7FFF - BIOS Video                   |-   Kernel
0x000C8000 --> 0x000FFFFF - BIOS                         |
0x00100000 --> 0x001FFFFF - Free                         |
0x00200000 --> 0x0020FFFF - Kernel                       |
0x00210000 --> 0x002FFFFF - Kernel Stack                -|

0x00400000 --> 0x004FFFFF - Free                        -|
0x00500000 --> 0x0050FFFF - Program1                     |- PageTable1
0x00510000 --> 0x0051FFFF - Program2                     |-    User
0x00520000 --> 0x007FFFFF - Free                        -|

0x00C00000 --> 0x00FFFFFF - kmalloc                     =|- PageTable3 - Heap
0x01000000 --> 0x013FFFFF - kmalloc                     =|- PageTable4 - Heap