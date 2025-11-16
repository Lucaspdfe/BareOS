org 0x00200000
bits 32

start:
    mov eax, 1        ; sys_write
    mov ebx, 1        ; stdout
    mov ecx, msg      ; ptr to text
    mov edx, msg_len  ; length
    int 0x80
    jmp $

msg     db "Hello, world from RING3!", 10
msg_len equ $ - msg
