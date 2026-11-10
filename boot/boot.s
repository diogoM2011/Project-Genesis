/*
 * Project Genesis - early boot path.
 *
 * The bootloader hands control to _start in 32-bit protected mode with paging
 * disabled. This file:
 *   1. carries the Multiboot2 header (primary protocol, used by GRUB);
 *   2. also carries a Multiboot1 header so QEMU's built-in -kernel loader can
 *      boot the very same kernel for quick local tests;
 *   3. installs an identity mapping for the first 1 GiB using 2 MiB pages;
 *   4. enables PAE + long mode and loads our own 64-bit GDT;
 *   5. jumps into 64-bit mode and calls kmain(magic, boot_info).
 *
 * Nothing here depends on a hosted C library or on the bootloader's GDT.
 */

.set MB2_MAGIC,        0xE85250D6
.set MB2_ARCH_I386,    0
.set MB1_MAGIC,        0x1BADB002
.set MB1_FLAGS,        0x00000003          /* align modules, request memory info */

/* ------------------------------------------------------------------------- */
/* Boot headers. They must sit at the very start of the image.                */
/* ------------------------------------------------------------------------- */

.section .multiboot, "a"
.p2align 3
mb2_header_start:
    .long MB2_MAGIC
    .long MB2_ARCH_I386
    .long mb2_header_end - mb2_header_start
    .long -(MB2_MAGIC + MB2_ARCH_I386 + (mb2_header_end - mb2_header_start))
    /* End tag: type 0, flags 0, size 8. A Multiboot2 header must end with it. */
    .p2align 3
    .short 0
    .short 0
    .long 8
mb2_header_end:

.p2align 2
mb1_header_start:
    .long MB1_MAGIC
    .long MB1_FLAGS
    .long -(MB1_MAGIC + MB1_FLAGS)
mb1_header_end:

/* ------------------------------------------------------------------------- */
/* 32-bit entry point                                                        */
/* ------------------------------------------------------------------------- */

.section .text.boot, "ax"
.code32
.global _start
.type _start, @function
_start:
    cli
    cld

    /* Preserve the bootloader hand-off values before using the registers. */
    movl %eax, boot_magic
    movl %ebx, boot_info

    /* The bootloader's stack is not part of our contract: use our own. */
    movl $stack_top, %esp
    xorl %ebp, %ebp

    /* Clear the page tables; do not rely on the loader zeroing .bss. */
    movl $pml4, %edi
    movl $1536, %ecx                       /* 3 tables x 512 entries */
    xorl %eax, %eax
    rep stosl

    /* PML4[0] -> PDPT, PDPT[0] -> PD. */
    movl $pdpt, %eax
    orl $0x3, %eax                         /* present | writable */
    movl %eax, pml4

    movl $pd, %eax
    orl $0x3, %eax
    movl %eax, pdpt

    /* Fill the page directory with 512 huge pages covering 0 - 1 GiB. */
    movl $pd, %edi
    xorl %ecx, %ecx
    movl $0x00000083, %eax                 /* present | writable | 2 MiB page */
1:
    movl %eax, (%edi,%ecx,8)
    movl $0, 4(%edi,%ecx,8)
    addl $0x200000, %eax
    incl %ecx
    cmpl $512, %ecx
    jne 1b

    /* Load the 64-bit GDT while we are still in 32-bit mode. */
    lgdt gdt64_ptr

    movl $pml4, %eax
    movl %eax, %cr3

    /* CR4.PAE = 1 */
    movl %cr4, %eax
    orl $(1 << 5), %eax
    movl %eax, %cr4

    /* EFER.LME = 1 */
    movl $0xC0000080, %ecx
    rdmsr
    orl $(1 << 8), %eax
    wrmsr

    /* CR0.PG = 1: long mode becomes active, we are now in compatibility mode. */
    movl %cr0, %eax
    orl $(1 << 31), %eax
    movl %eax, %cr0

    /* Far jump into a 64-bit code segment. */
    ljmp $0x08, $long_mode

/* ------------------------------------------------------------------------- */
/* 64-bit entry point                                                        */
/* ------------------------------------------------------------------------- */

.code64
long_mode:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw %ax, %fs
    movw %ax, %gs

    movq $stack_top, %rsp
    xorq %rbp, %rbp

    /* kmain(uint32_t magic, uint32_t boot_info); 32-bit moves zero-extend. */
    movl boot_magic, %edi
    movl boot_info, %esi
    call kmain

    /* kmain must not return; halt the CPU if it ever does. */
2:
    cli
    hlt
    jmp 2b

.size _start, . - _start

/* ------------------------------------------------------------------------- */
/* GDT                                                                       */
/* ------------------------------------------------------------------------- */

.section .rodata
.p2align 3
gdt64:
    .quad 0x0000000000000000               /* null descriptor */
    .quad 0x00AF9A000000FFFF               /* 0x08: 64-bit code, present, ring 0 */
    .quad 0x00CF92000000FFFF               /* 0x10: data, present, ring 0 */
gdt64_end:

/*
 * One pointer usable from both modes: a 32-bit lgdt reads the limit plus the
 * low 4 bytes of the base, a 64-bit lgdt reads the full 10 bytes.
 */
.p2align 3
gdt64_ptr:
    .word gdt64_end - gdt64 - 1
    .quad gdt64

/* ------------------------------------------------------------------------- */
/* Page tables and kernel stack                                              */
/* ------------------------------------------------------------------------- */

.section .bss
.p2align 12
pml4:
    .skip 4096
pdpt:
    .skip 4096
pd:
    .skip 4096

.p2align 3
boot_magic:
    .skip 4
boot_info:
    .skip 4

.p2align 4
stack_bottom:
    .skip 16384
stack_top:

.section .note.GNU-stack, "", @progbits
