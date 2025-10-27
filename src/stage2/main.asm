org 0x20000
bits 16

start:
    ; setup stack (avoid overlapping stage2 code)
    mov ax, 0x7C00      ; stack segment somewhere safe
    mov bp, ax
    mov sp, ax

    ; setup data segment
    mov ax, 0x2000              ; Segment that stage2 is loaded at
    mov ds, ax
    mov es, ax

    ; print message
    mov si, msg_hello
    call puts

    ; stop
    cli
    hlt
    jmp $               ; halt indefinitely

;
; Prints a string to the screen
; Params:
;   - ds:si points to string
;
puts:
    ; save registers we will modify
    push si
    push ax
    push bx

.loop:
    lodsb               ; loads next character in al
    or al, al           ; verify if next character is null?
    jz .done

    mov ah, 0x0E        ; call bios interrupt
    mov bh, 0           ; set page number to 0
    int 0x10

    jmp .loop

.done:
    pop bx
    pop ax
    pop si    
    ret

msg_hello: db "Hello, world from stage2!!!", 0
