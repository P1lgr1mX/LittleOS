#ifndef ARCH_X86_PAGING_H
#define ARCH_X86_PAGING_H

#include "types.h"

/* Virtual base address of the higher-half kernel (3GB boundary) */
#define KERNEL_VIRTUAL_BASE 0xC0000000

/* Address translation macros between virtual and physical memory in kernel space */
#define VIRTUAL_TO_PHYSICAL(addr) ((uint32_t)(addr) - KERNEL_VIRTUAL_BASE)
#define PHYSICAL_TO_VIRTUAL(addr) ((uint32_t)(addr) + KERNEL_VIRTUAL_BASE)

/* IA-32 page directory/table entry attribute flags */
#define PAGING_FLAG_PRESENT  0x01
#define PAGING_FLAG_WRITE    0x02
#define PAGING_FLAG_USER     0x04

#define PAGING_KERNEL_PAGE   (PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE)                    /* 0x03: Ring 0, Read/Write, Present */
#define PAGING_USER_PAGE     (PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE | PAGING_FLAG_USER) /* 0x07: Ring 3, Read/Write, Present */

/* Default stack top and execution entry point for userland processes */
#define USER_STACK_TOP       0xBFFFFFFB
#define USER_ENTRY_POINT     0x00000000

/* Initialize and synchronize higher-half paging configuration */
void paging_init(void);

/* Construct address space for userland process and map executable image */
void paging_setup_user_process(uint32_t module_start, uint32_t module_size);

/* Assembly routine in arch/x86/boot/loader.s executing iret into Ring 3 */
void enter_user_mode(uint32_t eip, uint32_t user_esp);

#endif /* ARCH_X86_PAGING_H */
