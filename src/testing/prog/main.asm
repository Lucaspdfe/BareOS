org 0x00500000
bits 32

SYS_EXIT        equ 1
SYS_WRITE       equ 4
SYS_OPEN        equ 5
SYS_GETDENTS    equ 141
SYS_GETDENTS64  equ 220

STDOUT equ 1

DT_DIR equ 4

O_RDONLY   equ 0
O_DIRECTORY equ 0200000

start:
    ; fd = open("/", O_RDONLY | O_DIRECTORY, 0)
    mov eax, SYS_OPEN
    mov ebx, path
    mov ecx, O_RDONLY | O_DIRECTORY
    xor edx, edx
    int 0x80
    cmp eax, 0
    jl exit
    mov [dirfd], eax

read_loop:
    ; try getdents64 first
    mov eax, SYS_GETDENTS64
    mov ebx, [dirfd]
    mov ecx, buf
    mov edx, bufsize
    int 0x80

    cmp eax, 0
    jg got_64
    cmp eax, 0
    jl try_32
    jmp exit

try_32:
    mov eax, SYS_GETDENTS
    mov ebx, [dirfd]
    mov ecx, buf
    mov edx, bufsize
    int 0x80

    test eax, eax
    jle exit
    mov byte [mode], 32
    jmp parse

got_64:
    mov byte [mode], 64

parse:
    mov esi, buf
    mov edi, eax

next_ent:
    cmp edi, 0
    jle read_loop

    cmp byte [mode], 64
    je ent64

; ---------------- getdents (32) ----------------
ent32:
    lea ecx, [esi + 10]          ; d_name
    call write_cstr

    ; d_type = last byte
    movzx eax, word [esi + 8]    ; d_reclen
    dec eax
    mov bl, [esi + eax]

    call maybe_dir

    movzx eax, word [esi + 8]
    add esi, eax
    sub edi, eax
    jmp next_ent

; ---------------- getdents64 ----------------
ent64:
    lea ecx, [esi + 19]          ; d_name
    call write_cstr

    mov bl, [esi + 18]           ; d_type
    call maybe_dir

    movzx eax, word [esi + 16]   ; d_reclen
    add esi, eax
    sub edi, eax
    jmp next_ent

; ---------------- helpers ----------------
maybe_dir:
    cmp bl, DT_DIR
    jne .nl

    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov ecx, dir_tag
    mov edx, dir_tag_len
    int 0x80
    ret

.nl:
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov ecx, nl
    mov edx, 1
    int 0x80
    ret

; write ECX = C-string
write_cstr:
    push ecx
    xor edx, edx
.len:
    cmp byte [ecx + edx], 0
    je .out
    inc edx
    jmp .len
.out:
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    pop ecx
    int 0x80
    ret

exit:
    mov eax, SYS_EXIT
    xor ebx, ebx
    int 0x80

; ---------------- data ----------------
path    db "/", 0
nl      db 10
dir_tag db " <DIR>", 10
dir_tag_len equ 7

mode    db 0
dirfd   dd 0
bufsize equ 1024
buf     times bufsize db 0
