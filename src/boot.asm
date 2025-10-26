org 0x7C00
bits 16


%define ENDL 0x0D, 0x0A


;
; FAT16 header
; 
jmp short start
nop

bdb_oem:                    db 'MTOO4048'           ; 8 bytes
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_cluster:    db 2
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 512
bdb_total_sectors:          dw 0                    ; More than 65536
bdb_media_descriptor_type:  db 0F8h                 ; F8 = HDD floppy disk
bdb_sectors_per_fat:        dw 255                  ; 9 sectors/fat
bdb_sectors_per_track:      dw 63
bdb_heads:                  dw 16
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 0x20000

; extended boot record
ebr_drive_number:           db 0x80                 ; 0x00 floppy, 0x80 hdd, useless
                            db 0                    ; reserved
ebr_signature:              db 29h
ebr_volume_id:              db 12h, 34h, 56h, 78h   ; serial number, value doesn't matter
ebr_volume_label:           db 'BARE OS    '        ; 11 bytes, padded with spaces
ebr_system_id:              db 'FAT16   '           ; 8 bytes

;
; Code goes here
;

start:
    ; setup data segments
    mov ax, 0           ; can't set ds/es directly
    mov ds, ax
    mov es, ax
    
    ; setup stack
    mov ss, ax
    mov sp, 0x7C00              ; stack grows downwards from where we are loaded in memory

    ; some BIOSes might start us at 07C0:0000 instead of 0000:7C00, make sure we are in the
    ; expected location
    push es
    push word .after
    retf

.after:
    mov si, msg_hello
    call puts

    cli
    hlt

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

msg_hello: db "Hello, world!", 0

times 510-($-$$) db 0
dw 0AA55h                   ; Little endian (0x55, 0xaa)