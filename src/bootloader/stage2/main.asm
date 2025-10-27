org 0x20000

start:
    [bits 16]
    ; setup stack (avoid overlapping stage2 code)
    mov ax, 0x7C00      ; stack segment somewhere safe
    mov bp, ax
    mov sp, ax

    ; setup data segment
    mov ax, 0x2000              ; Segment that stage2 is loaded at
    mov ds, ax
    mov es, ax

    call get_a20_state
    cmp ax, 1
    je .after_a20

    ; 1 - Enable A20 using Fast A20 Gate method
    in al, 0x92
    or al, 2
    out 0x92, al

.after_a20:
    [bits 16]
    call INTs_disable   ; 2 - Disable Interrupts
    lgdt [g_GDTDesc]    ; 3 - Load the GDT
    mov eax, cr0 
    or al, 1            ; 4 - set PE (Protection Enable) bit in CR0 (Control Register 0)
    mov cr0, eax
    jmp dword 08h:.pmode

.pmode:
    ; we are now in protected mode!
    [bits 32]
    
    ; setup segment registers
    mov ax, 0x10
    mov ds, ax
    mov ss, ax

    ; prints 'hi'
    mov word [ScreenBuffer + 0], 'h'
    mov word [ScreenBuffer + 1], 0x07       ; light gray
    mov word [ScreenBuffer + 2], 'i'
    mov word [ScreenBuffer + 3], 0x07       ; light gray

    cli
    hlt

; void INTs_disable() {
;     cli();
;     outb(0x70, inb(0x70) | 0x80);
;     inb(0x71);
; }
INTs_disable:
    [bits 16]
    cli                 ; disable normal interrupts
    ; disable NMIs
    in   al, 0x70       ; read from port 0x70
    or   al, 0x80       ; set bit 7 (disable NMI)
    out  0x70, al       ; write back to port 0x70
    in   al, 0x71       ; dummy read to port 0x71
    ret

;	out:
;		ax - state (0 - disabled, 1 - enabled)
get_a20_state:
    [bits 16]
	pushf
	push si
	push di
	push ds
	push es
	cli

	mov ax, 0x0000					;	0x0000:0x0500(0x00000500) -> ds:si
	mov ds, ax
	mov si, 0x0500

	not ax						;	0xffff:0x0510(0x00100500) -> es:di
	mov es, ax
	mov di, 0x0510

	mov al, [ds:si]					;	save old values
	mov byte [.BufferBelowMB], al
	mov al, [es:di]
	mov byte [.BufferOverMB], al

	mov ah, 1
	mov byte [ds:si], 0
	mov byte [es:di], 1
	mov al, [ds:si]
	cmp al, [es:di]					;	check byte at address 0x0500 != byte at address 0x100500
	jne .exit
	dec ah
.exit:
    [bits 16]
	mov al, [.BufferBelowMB]
	mov [ds:si], al
	mov al, [.BufferOverMB]
	mov [es:di], al
	shr ax, 8					;	move result from ah to al register and clear ah
	sti
	pop es
	pop ds
	pop di
	pop si
	popf
	ret
	

.BufferBelowMB:	db 0
.BufferOverMB	db 0

g_GDT:      ; NULL descriptor (offset 0x00)
            dq 0

            ; 32-bit code segment (offset 0x08)
            dw 0FFFFh                   ; limit (bits 0-15) = 0xFFFFF for full 32-bit range
            dw 0                        ; base (bits 0-15) = 0x0
            db 0                        ; base (bits 16-23)
            db 10011010b                ; access (present, ring 0, code segment, executable, direction 0, readable)
            db 11001111b                ; granularity (4k pages, 32-bit pmode) + limit (bits 16-19)
            db 0                        ; base high

            ; 32-bit data segment (offset 0x10)
            dw 0FFFFh                   ; limit (bits 0-15) = 0xFFFFF for full 32-bit range
            dw 0                        ; base (bits 0-15) = 0x0
            db 0                        ; base (bits 16-23)
            db 10010010b                ; access (present, ring 0, data segment, executable, direction 0, writable)
            db 11001111b                ; granularity (4k pages, 32-bit pmode) + limit (bits 16-19)
            db 0                        ; base high

            ; 16-bit code segment (offset 0x18)
            dw 0FFFFh                   ; limit (bits 0-15) = 0xFFFFF
            dw 0                        ; base (bits 0-15) = 0x0
            db 0                        ; base (bits 16-23)
            db 10011010b                ; access (present, ring 0, code segment, executable, direction 0, readable)
            db 00001111b                ; granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
            db 0                        ; base high

            ; 16-bit data segment (offset 0x20)
            dw 0FFFFh                   ; limit (bits 0-15) = 0xFFFFF
            dw 0                        ; base (bits 0-15) = 0x0
            db 0                        ; base (bits 16-23)
            db 10010010b                ; access (present, ring 0, data segment, executable, direction 0, writable)
            db 00001111b                ; granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
            db 0                        ; base high

g_GDTDesc:  dw g_GDTDesc - g_GDT - 1    ; limit = size of GDT
            dd g_GDT                    ; address of GDT

ScreenBuffer                        equ 0xB8000
