#include "types.h"
#include "drivers/framebuffer.h"
#include "drivers/serial.h"
#include "drivers/pic.h"
#include "drivers/keyboard.h"
#include "arch/x86/io.h"
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#include "arch/x86/mmu/paging.h"
#include "kernel/syscall.h"
#include "kernel/kheap.h"
#include "kernel/multiboot.h"

/* Kernel memory boundary symbols defined in link.ld */
extern uint32_t kernel_physical_start;
extern uint32_t kernel_physical_end;
extern uint32_t kernel_virtual_start;
extern uint32_t kernel_virtual_end;

/* Kernel bootstrap stack symbols defined in arch/x86/boot/loader.s */
extern uint8_t kernel_stack[];
extern const uint32_t KERNEL_STACK_SIZE;

/* Calculate the length of a null-terminated string */
static uint32_t strlen(const char *str)
{
    uint32_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/* Helper function to output a null-terminated string to the framebuffer */
static void print(const char *str)
{
    fb_write(str, strlen(str));
}

/* Helper function to format and print a 32-bit integer in hexadecimal notation */
static void print_hex(uint32_t n)
{
    char buf[11];
    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        int nibble = (n >> (i * 4)) & 0xF;
        buf[9 - i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
    }
    buf[10] = '\0';
    print(buf);
}

int kmain(uint32_t mb_info_addr)
{
    /* Clear console screen and reset cursor to (0, 0) */
    fb_clear();    

    /* Display kernel welcome banner */
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    print("=================================================================\n");
    print("                 Welcome to Little Kernel v0.1.0                 \n");
    print("                 Developed by AmiyaSokiya (amiyasociu@gmail.com)  \n");
    print("=================================================================\n\n");

    fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
    print("[ OK ] Framebuffer driver initialized successfully.\n");
    print("[ OK ] Cursor auto-advance enabled.\n");
    print("[ OK ] Hardware cursor synchronization active.\n\n");

    /* Initialize and verify serial communication driver (COM1: 115200 8N1, IRQ 4) */
    fb_set_color(FB_LIGHT_MAGENTA, FB_BLACK);
    serial_init();

    /* Transmit diagnostic string over COM1 */
    serial_write_char(SERIAL_COM1_BASE, 'A');
    serial_write_char(SERIAL_COM1_BASE, 'E');
    const char *serial_msg = " [Serial COM1] LittleOS Serial Driver initialized successfully!\r\n";
    serial_write(SERIAL_COM1_BASE, serial_msg, strlen(serial_msg));

    print("[ OK ] Serial driver initialized with IRQ 4 successfully!\n");

    /* Initialize Global Descriptor Table (GDT) and TSS */
    gdt_init();
    print("[ OK ] GDT initialized & loaded successfully.\n");

    /* Initialize Interrupt Descriptor Table (IDT) */
    idt_init();
    print("[ OK ] IDT initialized & loaded with 48 interrupt gates.\n");

    /* Register System Call interrupt gate at INT 0x80 */
    syscall_init();
    print("[ OK ] Syscall handler registered at INT 0x80 (DPL=3).\n");

    /* Remap 8259 Programmable Interrupt Controller (PIC) */
    pic_remap();
    print("[ OK ] PIC remapped (Master: 0x20, Slave: 0x28).\n");

    /* Initialize PS/2 keyboard controller driver */
    keyboard_init();
    print("[ OK ] PS/2 Keyboard driver initialized on IRQ 1.\n");

    /* Synchronize higher-half paging configuration */
    paging_init();
    print("[ OK ] Higher-Half Paging active (Kernel at 0xC0100000, 3GB Virtual Base).\n");

    /* Initialize Kernel Dynamic Bitmap Heap */
    kheap_init();
    print("[ OK ] Kernel Bitmap Heap initialized (256 KB, 32B block size).\n");

    /* Perform diagnostic allocation and deallocation test on heap */
    fb_set_color(FB_LIGHT_BROWN, FB_BLACK);
    char *test_alloc = (char *)kmalloc(128);
    if (test_alloc) {
        const char *test_str = "Heap allocation test: OK!\n";
        for (uint32_t i = 0; i < strlen(test_str); i++) {
            test_alloc[i] = test_str[i];
        }
        test_alloc[strlen(test_str)] = '\0';
        print("  -> ");
        print(test_alloc);
        kheap_print_stats();
        kfree(test_alloc);
        print("  -> Freed test block. ");
        kheap_print_stats();
    }

    /* Print kernel memory layout addresses */
    uint32_t phys_start = (uint32_t)&kernel_physical_start;
    uint32_t phys_end   = (uint32_t)&kernel_physical_end;
    uint32_t virt_start = (uint32_t)&kernel_virtual_start;
    uint32_t virt_end   = (uint32_t)&kernel_virtual_end;

    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    print("[ MEM ] Kernel Physical: ");
    print_hex(phys_start);
    print(" -> ");
    print_hex(phys_end);
    print("\n[ MEM ] Kernel Virtual : ");
    print_hex(virt_start);
    print(" -> ");
    print_hex(virt_end);
    print("\n");

    /* Enable hardware interrupts */
    enable_interrupts();
    print("[ OK ] CPU interrupts enabled (sti).\n\n");

    /* Prepare privilege transition to User Mode (Ring 3) */
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    print("=================================================================\n");
    print("[ OK ] Kernel initialization complete. Transitioning to User Mode...\n\n");
    fb_set_color(FB_WHITE, FB_BLACK);

    multiboot_info_t *mbinfo = (multiboot_info_t *) mb_info_addr;

    /* Verify Multiboot modules before loading userland program */
    if ((mbinfo->flags & MULTIBOOT_INFO_MODS) && mbinfo->mods_count > 0) {
        multiboot_module_t *mod = (multiboot_module_t *) mbinfo->mods_addr;
        uint32_t mod_start = mod->mod_start;
        uint32_t mod_size = mod->mod_end - mod->mod_start;

        fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
        print("[ OK ] GRUB module found! Configuring User Mode (Ring 3)...\n");

        /* Configure userland virtual address space */
        paging_setup_user_process(mod_start, mod_size);
        print("[ OK ] User Page Directory & Tables mapped with U/S = 1.\n");

        /* Set kernel stack pointer in TSS */
        tss_set_kernel_stack((uint32_t)kernel_stack + 4096);
        print("[ OK ] TSS Ring 0 stack pointer configured.\n");

        /* Initiate jump to Ring 3 */
        fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
        print("[ INFO ] Executing iret to drop CPU privilege into Ring 3...\n");
        const char *serial_user_msg = "[User Mode] Switching to Ring 3 via iret (CS=0x1B, SS=0x23, EIP=0x0)...\r\n";
        serial_write(SERIAL_COM1_BASE, serial_user_msg, strlen(serial_user_msg));

        /* Drop to Ring 3: EIP = 0x00000000, ESP = 0xBFFFFFFB */
        enter_user_mode(USER_ENTRY_POINT, USER_STACK_TOP);
    } else {
        fb_set_color(FB_LIGHT_RED, FB_BLACK);
        print("[ WARN ] No Multiboot module found.\n");
    }

    return 0;
}