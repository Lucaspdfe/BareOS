[bits 32]

; void __attribute__((cdecl)) GDT_Load(GDTDescriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment);
global GDT_Load
GDT_Load:
    ; make new call frame
    push ebp
    mov ebp, esp

    ; load gdt
    mov eax, [ebp + 8]
    lgdt [eax]

    ; reload code and data segment
    mov eax, [ebp + 12]
    push eax
    push .reload_cs
    retf
.reload_cs:
    mov ax, [ebp + 16]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; restore call frame
    mov esp, ebp
    pop ebp
    ret
