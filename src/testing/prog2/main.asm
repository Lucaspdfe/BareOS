org 0x00210000
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

msg     db "This is a test code from the second program!", 10

msg_len equ $ - msg
