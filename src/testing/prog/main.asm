org 0x00200000
bits 32

%define SYS_exit 1
%define SYS_execve 11

start:
    ; filename and argv in data
    mov eax, SYS_execve
    mov ebx, filename    ; char *filename
    mov ecx, argv        ; char **argv
    xor edx, edx         ; envp = NULL
    int 0x80

    ; if execve returns, exit with error
    mov eax, SYS_exit
    mov ebx, 1
    int 0x80

; --------------------
; Data
; --------------------
filename:
    db "/usr/prog2.bin",0
    align 4
argv:
    dd filename
    dd 0
