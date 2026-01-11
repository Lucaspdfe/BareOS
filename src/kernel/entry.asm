bits 32

section .entry

extern kmain
global start

start:
    pop eax         ; pop return address (not needed)
    pop eax         ; pop tags address

    mov esp, 0x00300000
    mov ebp, 0

    push eax
    call kmain

    hlt
    jmp $-1
