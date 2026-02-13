[bits 32]

; void __attribute__((cdecl)) IDT_Load(IDTDescriptor* Descriptor);
global IDT_Load
IDT_Load:
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    lidt [eax]

    mov esp, ebp
    pop ebp
    ret