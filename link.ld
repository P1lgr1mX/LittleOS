/* 
 * link.ld: Linker script for Higher-Half Kernel (AetherOS)
 *
 * Architecture and Layout:
 * 1. The bootloader (GRUB) loads the kernel image into physical RAM at 1MB (0x00100000).
 *    Therefore, the Load Memory Address (LMA) of the bootstrap section begins at 0x00100000.
 * 2. All kernel instructions, symbols, and pointers are linked to execute within the
 *    higher-half virtual memory address space (VMA) starting at 3GB + 1MB (0xC0100000).
 * 3. The entry point is `_start` in the `.boot` section at 1MB physical memory,
 *    allowing the CPU to initialize paging prior to MMU activation without faults.
 */

ENTRY(_start)

SECTIONS
{
    /* -------------------------------------------------------------
     * Stage 1: Bootstrap initialization section
     * VMA = LMA = 0x00100000 (1MB physical memory)
     * ------------------------------------------------------------- */
    . = 0x00100000;
    kernel_physical_start = .;
    kernel_virtual_start = . + 0xC0000000;
    .boot : {
        *(.multiboot)   /* Multiboot header (must reside within first 8KB of binary) */
        *(.boot)        /* Bootstrap assembly: sets up initial page directory and enables paging */
    }

    /* -------------------------------------------------------------
     * Stage 2: Higher-Half Kernel Transition
     * Shift Virtual Memory Address (VMA) to 3GB base: 0xC0000000
     * Load Memory Address (LMA) is defined via AT(ADDR(...) - 0xC0000000)
     * ------------------------------------------------------------- */
    . += 0xC0000000;

    /* Executable kernel code (.text), aligned to 4KB page boundary */
    .text ALIGN (4096) : AT(ADDR(.text) - 0xC0000000) {
        *(.text)
    }

    /* Read-only data (.rodata) */
    .rodata ALIGN (4096) : AT(ADDR(.rodata) - 0xC0000000) {
        *(.rodata*)
    }

    /* Initialized read-write data (.data) */
    .data ALIGN (4096) : AT(ADDR(.data) - 0xC0000000) {
        *(.data)
    }

    /* Uninitialized read-write data (.bss) and common symbols */
    .bss ALIGN (4096) : AT(ADDR(.bss) - 0xC0000000) {
        *(COMMON)
        *(.bss)
    }

    kernel_virtual_end = .; 
    kernel_physical_end = . - 0xC0000000;
}
