org 0x00200000
bits 32

%define SYS_exit     1
%define SYS_write    4
%define SYS_open     5
%define SYS_getdents 141

%define STDOUT 1
%define MAX_ENTRIES  100        ; <-- entry limit

start:
    ; open("/")
    mov eax, SYS_open
    mov ebx, path
    int 0x80
    cmp eax, 0
    js exit
    mov [dirfd], eax

read_dir:
    cmp dword [entry_count], MAX_ENTRIES
    jge exit

    ; getdents(fd, buf, bufsize)
    mov eax, SYS_getdents
    mov ebx, [dirfd]
    mov ecx, dentbuf
    mov edx, dentbuf_size
    int 0x80

    test eax, eax
    jz exit
    js exit

    mov esi, dentbuf
    mov edi, eax     ; bytes returned

next_entry:
    cmp edi, 0
    jle read_dir

    cmp dword [entry_count], MAX_ENTRIES
    jge exit

    ; d_reclen (in bytes)
    movzx ecx, word [esi + 8]
    push ecx                ; preserve reclen while we compute name length

    ; compute max name length = reclen - 10 (header size)
    mov edx, ecx
    sub edx, 10

    ; pointer to name
    lea ebx, [esi + 10]

    ; find length (limit to edx)
    xor eax, eax
    cmp edx, 0
    je .no_name
.find_len:
    cmp byte [ebx + eax], 0
    je .got_len
    inc eax
    cmp eax, edx
    jl .find_len
.got_len:
    ; write name (ecx=ptr, edx=len)
    mov ecx, ebx
    mov edx, eax
    mov eax, SYS_write
    mov ebx, STDOUT
    int 0x80

    ; write newline
    mov eax, SYS_write
    mov ebx, STDOUT
    mov ecx, newline
    mov edx, 1
    int 0x80
.no_name:
    pop ecx                 ; restore reclen

    inc dword [entry_count]

    add esi, ecx
    sub edi, ecx
    jmp next_entry

exit:
    mov eax, SYS_exit
    xor ebx, ebx
    int 0x80

; --------------------------------
; strlen(ecx = ptr) -> eax = length
; --------------------------------
strlen:
    xor eax, eax
.len:
    cmp byte [ecx + eax], 0
    je .done
    inc eax
    jmp .len
.done:
    ret

; --------------------------------
; Data
; --------------------------------
path        db "/usr/", 0
newline     db 10
dirfd       dd 0
entry_count dd 0

dentbuf_size equ 1024
dentbuf     times dentbuf_size db 0
