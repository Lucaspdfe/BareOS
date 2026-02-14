; main/entry.asm

bits 32

section .text

extern kmain
global start

start:
    mov esp, 0x00300000
    mov ebp, 0

    push eax

    call kmain

    cli
    hlt
