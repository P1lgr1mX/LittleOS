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

/* Các nhãn ranh giới bộ nhớ kernel từ link.ld */
extern uint32_t kernel_physical_start;
extern uint32_t kernel_physical_end;
extern uint32_t kernel_virtual_start;
extern uint32_t kernel_virtual_end;

/* Stack kernel được cấp trong arch/x86/boot/loader.s */
extern uint8_t kernel_stack[];
extern const uint32_t KERNEL_STACK_SIZE;

/* Hàm tính độ dài chuỗi ký tự kết thúc bằng '\0' */
static uint32_t strlen(const char *str)
{
    uint32_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/* Hàm phụ trợ in chuỗi ra màn hình dùng fb_write() */
static void print(const char *str)
{
    fb_write(str, strlen(str));
}

/* Hàm phụ trợ in số nguyên 32-bit dưới dạng Hexadecimal (0x12345678) */
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

int kmain(uint32_t ebx)
{
    /* Xóa sạch màn hình và đưa con trỏ về (0, 0) */
    fb_clear();    

    /* In tiêu đề với màu sắc sinh động */
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    print("=================================================================\n");
    print("                 Welcome to Little Kernel v0.1.0                 \n");
    print("                 Developed by AmiyaSokiya (amiyasociu@gmail.com)  \n");
    print("=================================================================\n\n");

    fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
    print("[ OK ] Framebuffer driver initialized successfully.\n");
    print("[ OK ] Cursor auto-advance enabled.\n");
    print("[ OK ] Hardware cursor synchronization active.\n\n");

    /* Khởi tạo và kiểm tra Serial driver (COM1: 115200 8N1) */
    fb_set_color(FB_LIGHT_MAGENTA, FB_BLACK);
    serial_config_baud_rate(SERIAL_COM1_BASE, 115200);
    serial_config_line(SERIAL_COM1_BASE);
    serial_config_buffers(SERIAL_COM1_BASE);
    serial_config_modem(SERIAL_COM1_BASE);

    /* Ghi dữ liệu kiểm tra ra cổng serial COM1 */
    serial_write_char(SERIAL_COM1_BASE, 'A');
    serial_write_char(SERIAL_COM1_BASE, 'E');
    const char *serial_msg = " [Serial COM1] LittleOS Serial Driver initialized successfully!\r\n";
    serial_write(SERIAL_COM1_BASE, serial_msg, strlen(serial_msg));

    print("[ OK ] Serial driver initialized successfully!\n");

    /* Khởi tạo phân đoạn bộ nhớ GDT */
    gdt_init();
    print("[ OK ] GDT initialized & loaded successfully.\n");

    /* Khởi tạo bảng phân phối ngắt IDT */
    idt_init();
    print("[ OK ] IDT initialized & loaded with 48 interrupt gates.\n");

    /* Đăng ký System Call handler tại ngắt 0x80 (128) */
    syscall_init();
    print("[ OK ] Syscall handler registered at INT 0x80 (DPL=3).\n");

    /* Khởi tạo bộ điều khiển ngắt khả trình PIC */
    pic_remap();
    print("[ OK ] PIC remapped (Master: 0x20, Slave: 0x28).\n");

    /* Khởi tạo trình điều khiển bàn phím PS/2 */
    keyboard_init();
    print("[ OK ] PS/2 Keyboard driver initialized on IRQ 1.\n");

    /* Khởi tạo phân trang (Paging) */
    paging_init();
    print("[ OK ] Higher-Half Paging active (Kernel at 0xC0100000, 3GB Virtual Base).\n");

    /* Khởi tạo Kernel Bitmap Heap */
    kheap_init();
    print("[ OK ] Kernel Bitmap Heap initialized (256 KB, 32B block size).\n");

    /* Kiểm tra kiểm thử cấp phát động kmalloc / kfree */
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

    /* Hiển thị phạm vi bộ nhớ của Kernel */
    uint32_t p_start = (uint32_t)&kernel_physical_start;
    uint32_t p_end   = (uint32_t)&kernel_physical_end;
    uint32_t v_start = (uint32_t)&kernel_virtual_start;
    uint32_t v_end   = (uint32_t)&kernel_virtual_end;

    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    print("[ MEM ] Kernel Physical: ");
    print_hex(p_start);
    print(" -> ");
    print_hex(p_end);
    print("\n[ MEM ] Kernel Virtual : ");
    print_hex(v_start);
    print(" -> ");
    print_hex(v_end);
    print("\n");

    /* Bật ngắt CPU */
    enable_interrupts();
    print("[ OK ] CPU interrupts enabled (sti).\n\n");

    /* Sẵn sàng nhận thao tác gõ phím từ người dùng */
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    print("=================================================================\n");
    print("Keyboard input active! You can type in the console now:\n");
    print("LittleOS> ");
    fb_set_color(FB_WHITE, FB_BLACK);

    multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;

    /* Kiểm tra cờ Multiboot và số lượng module hợp lệ trước khi gọi */
    if ((mbinfo->flags & MULTIBOOT_INFO_MODS) && mbinfo->mods_count > 0) {
        multiboot_module_t *mod = (multiboot_module_t *) mbinfo->mods_addr;
        uint32_t mod_start = mod->mod_start;
        uint32_t mod_size = mod->mod_end - mod->mod_start;

        fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
        print("[ OK ] GRUB module found! Configuring User Mode (Ring 3)...\n");

        /* Cấu hình phân trang cho User Mode */
        paging_setup_user_process(mod_start, mod_size);
        print("[ OK ] User Page Directory & Tables mapped with U/S = 1.\n");

        /* Cài đặt Kernel Stack vào TSS */
        tss_set_kernel_stack((uint32_t)kernel_stack + 4096);
        print("[ OK ] TSS Ring 0 stack pointer configured.\n");

        /* Chuẩn bị bước nhảy sang User Mode */
        fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
        print("[ INFO ] Executing iret to drop CPU privilege into Ring 3...\n");
        const char *serial_user_msg = "[User Mode] Switching to Ring 3 via iret (CS=0x1B, SS=0x23, EIP=0x0)...\r\n";
        serial_write(SERIAL_COM1_BASE, serial_user_msg, strlen(serial_user_msg));

        /* Nhảy sang Ring 3: EIP = 0x00000000, ESP = 0xBFFFFFFB */
        enter_user_mode(USER_ENTRY_POINT, USER_STACK_TOP);
    } else {
        fb_set_color(FB_LIGHT_RED, FB_BLACK);
        print("[ WARN ] No Multiboot module found.\n");
    }

    return 0;
}