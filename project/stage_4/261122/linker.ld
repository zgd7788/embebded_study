ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 0xC0008000;

    .text : ALIGN(4)
    {
        KEEP(*(.text.start))
        *(.text .text.*)
    } :text

    .rodata : ALIGN(4)
    {
        *(.rodata .rodata.*)
    } :text

    .data : ALIGN(4)
    {
        *(.data .data.*)
    } :data

    .bss (NOLOAD) : ALIGN(4)
    {
        __bss_start__ = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } :data

    . = ALIGN(8);
    __image_end__ = .;
    __stack_top__ = 0xC1000000;

    /DISCARD/ :
    {
        *(.comment)
        *(.note .note.*)
        *(.eh_frame .eh_frame.*)
    }
}
