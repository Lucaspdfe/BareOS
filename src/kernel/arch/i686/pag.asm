bits 32

section .text

extern PageDirectory
global i686_PAG_Setup

i686_PAG_Setup:
    cli

    mov eax, PageDirectory
    mov cr3, eax          ; load page directory

    mov eax, cr0
    or  eax, 0x80000000   ; set PG bit
    mov cr0, eax

    sti

    ret
