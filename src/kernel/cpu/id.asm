[bits 32]

; bool __attribute__((cdecl)) ID_CheckSupported();
global ID_CheckSupported
ID_CheckSupported:
    push ebp
    mov ebp, esp

    pushfd                      ; Save  EFLAGS
    pushfd                      ; Store EFLAGS
    xor dword [esp], 0x00200000 ; Invert the ID bit in stored EFLAGS
    popfd                       ; Load stored EFLAGS (With ID bit inverted)
    pushfd                      ; Store EFLAGS again (ID bit may or may not be inverted)
    pop eax                     ; eax = modified EFLAGS (ID bit may or may not be inverted)
    xor eax, [esp]              ; eax = whichever bits were changed
    popfd
    and eax, 0x00200000         ; eax = zero if ID bit can't be changed, else non-zero
    test eax, eax
    jz .not_supported
    mov eax, 1
    jmp .end
.not_supported:
    mov eax, 0
.end:
    mov esp, ebp
    pop ebp
    ret
