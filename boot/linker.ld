/*
 * Project Genesis - linker script.
 *
 * The kernel is linked at the traditional 1 MiB mark and identity mapped by the
 * boot path, so code can use absolute addresses without a higher-half offset.
 * The boot headers must be the first bytes of the first loadable section.
 */

ENTRY(_start)

SECTIONS
{
    . = 1M;

    .boot : ALIGN(4K)
    {
        KEEP(*(.multiboot))
    }

    .text : ALIGN(4K)
    {
        *(.text.boot)
        *(.text .text.*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata .rodata.*)
    }

    .data : ALIGN(4K)
    {
        *(.data .data.*)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss .bss.*)
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame .eh_frame_hdr)
        *(.note .note.*)
    }
}
