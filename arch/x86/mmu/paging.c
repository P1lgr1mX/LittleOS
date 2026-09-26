#include "arch/x86/mmu/paging.h"

/* Bootstrap paging structures statically defined and 4KB-aligned in arch/x86/boot/loader.s */
extern uint32_t boot_page_directory[1024];
extern uint32_t boot_page_table1[1024];

/* Assembly routine in arch/x86/boot/loader.s for reloading CR3 */
extern void load_page_directory(uint32_t cr3);

/* Maximum physical capacity allocated for userland binary image (16 pages * 4KB = 64KB) */
#define USER_CODE_MAX_PAGES 16

/* Page frames and page tables for userland processes, aligned to 4KB boundaries */
static uint32_t  user_page_directory[1024]                  __attribute__((aligned(4096)));
static uint32_t  user_code_page_table[1024]                 __attribute__((aligned(4096)));
static uint32_t  user_stack_page_table[1024]                __attribute__((aligned(4096)));
static uint8_t   user_code_pages[USER_CODE_MAX_PAGES][4096] __attribute__((aligned(4096)));
static uint8_t   user_stack_page[4096]                      __attribute__((aligned(4096)));

/**
 * paging_init:
 * Verifies and establishes paging structures for the Higher-Half Kernel:
 * - Entry 0 (virtual 0MB - 4MB): Identity-mapped to physical 0MB - 4MB.
 * - Entry 768 (virtual 3GB - 3GB+4MB): Higher-half mapped to physical 0MB - 4MB.
 */
void paging_init(void)
{
    /* Compute physical address of boot_page_table1 by subtracting KERNEL_VIRTUAL_BASE (0xC0000000) */
    uint32_t pt_phys = VIRTUAL_TO_PHYSICAL(boot_page_table1);

    /*
     * Bit 0: Present = 1 (page table present in RAM)
     * Bit 1: Read/Write = 1 (read/write operations enabled)
     * Bit 2: Supervisor = 0 (accessible only by Ring 0 kernel)
     */
    boot_page_directory[0]   = pt_phys | PAGING_KERNEL_PAGE;
    boot_page_directory[768] = pt_phys | PAGING_KERNEL_PAGE;
}

/**
 * paging_setup_user_process:
 * Configures the virtual address space and loads the user process image:
 * 1. 64KB (16 pages) mapped at virtual address 0x00000000 (.text, .data, .bss).
 * 2. User stack mapped at virtual address 0xBFFFFFFB (Page Directory entry 767).
 * 3. Kernel space preserved at 0xC0000000 (Entry 768) with supervisor privileges.
 * 4. Loads the physical address of user_page_directory into CR3.
 */
void paging_setup_user_process(uint32_t module_start, uint32_t module_size)
{
    /* Clear page directories, page tables, and memory frames */
    for (int i = 0; i < 1024; i++) {
        user_page_directory[i]   = 0;
        user_code_page_table[i]  = 0;
        user_stack_page_table[i] = 0;
    }
    for (int p = 0; p < USER_CODE_MAX_PAGES; p++) {
        for (int i = 0; i < 4096; i++) {
            user_code_pages[p][i] = 0;
        }
    }
    for (int i = 0; i < 4096; i++) {
        user_stack_page[i] = 0;
    }

    /* Copy executable image from loaded Multiboot module into allocated user page frames */
    uint8_t *src = (uint8_t *)module_start;
    for (uint32_t p = 0; p < USER_CODE_MAX_PAGES; p++) {
        uint32_t offset = p * 4096;
        if (offset < module_size) {
            uint32_t to_copy = 4096;
            if (offset + to_copy > module_size) {
                to_copy = module_size - offset;
            }
            for (uint32_t i = 0; i < to_copy; i++) {
                user_code_pages[p][i] = src[offset + i];
            }
        }

        /* Map 64KB User Code/Data/BSS starting at virtual address 0x00000000 */
        user_code_page_table[p] = VIRTUAL_TO_PHYSICAL(user_code_pages[p]) | PAGING_USER_PAGE;
    }
    user_page_directory[0] = VIRTUAL_TO_PHYSICAL(user_code_page_table) | PAGING_USER_PAGE;

    /* 2. Map User Stack at virtual address 0xBFFFFFFB */
    user_stack_page_table[1023] = VIRTUAL_TO_PHYSICAL(user_stack_page) | PAGING_USER_PAGE;
    user_page_directory[767]    = VIRTUAL_TO_PHYSICAL(user_stack_page_table) | PAGING_USER_PAGE;

    /* 3. Retain Higher-Half kernel mapping (0xC0000000 and above) */
    user_page_directory[768] = boot_page_directory[768];

    /* 4. Switch page directory by loading physical address into CR3 */
    load_page_directory(VIRTUAL_TO_PHYSICAL(user_page_directory));
}
