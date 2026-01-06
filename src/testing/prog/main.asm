org 0x00200000
bits 32

SYS_EXIT     equ 1
SYS_WRITE    equ 4
SYS_OPEN     equ 5
SYS_GETDENTS equ 141

STDOUT equ 1

start:
    ; fd = open("/")
    mov eax, SYS_OPEN
    mov ebx, path
    int 0x80
    cmp eax, 0
    jl exit
    mov [dirfd], eax

read_loop:
    ; n = getdents(fd, buf, bufsize)
    mov eax, SYS_GETDENTS
    mov ebx, [dirfd]
    mov ecx, buf
    mov edx, bufsize
    int 0x80

    test eax, eax
    jle exit          ; 0 = EOF, <0 = error

    mov esi, buf
    mov edi, eax      ; bytes returned

next_ent:
    cmp edi, 0
    jle read_loop

    ; d_name starts at offset 10
    lea ecx, [esi + 10]

    ; write name
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov edx, 11       ; max name length (safe upper bound)
    int 0x80

    ; write newline
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov ecx, nl
    mov edx, 1
    int 0x80

    ; advance by d_reclen (offset 8)
    movzx eax, word [esi + 8]
    add esi, eax
    sub edi, eax
    jmp next_ent

exit:
    mov eax, SYS_EXIT
    xor ebx, ebx
    int 0x80

; ---------------- data ----------------

path    db "/usr", 0
nl      db 10

dirfd   dd 0
bufsize equ 512
buf     times bufsize db 0