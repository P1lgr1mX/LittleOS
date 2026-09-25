ENTRY(_start)

SECTIONS
{
    . = 0x00000000;

    .text : {
        *(.text.entry)
        *(.text*)
    }

    .rodata ALIGN(4) : {
        *(.rodata*)
    }

    .data ALIGN(4) : {
        *(.data*)
    }

    .bss ALIGN(4) : {
        *(COMMON)
        *(.bss*)
    }

    /DISCARD/ : {
        *(.eh_frame)
        *(.comment)
    }
}
