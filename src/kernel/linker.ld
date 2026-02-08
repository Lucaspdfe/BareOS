ENTRY(start)
OUTPUT_FORMAT("binary")
phys = 0x00100000;

SECTIONS
{
    . = phys;

    .text   : { __text_start = .; *(.text*) }

    .rodata : { __rodata_start = .; *(.rodata*) }

    .data   : {
        __data_start = .;
        *(.data*)
        __data_end = .;
    }

    .bss    : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    }

    __end = .;
}
