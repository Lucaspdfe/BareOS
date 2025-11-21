org 0x00200000
bits 32

start:
    mov eax, 4        ; sys_write
    mov ebx, 1        ; stdout
    mov ecx, msg      ; ptr to text
    mov edx, msg_len  ; length
    int 0x80
    mov eax, 1
    mov ebx, 0
    int 0x80
    jmp $

msg     db "Hello, world from RING3 (Program 2)!", 10
msg_len equ $ - msg
