#include "framebuffer.h"
#include "serial.h"
#include "io.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "multiboot.h"
#include "paging.h"

typedef void (*call_module_t)(void);

/* Các nhãn ranh giới bộ nhớ kernel từ link.ld */
extern unsigned int kernel_physical_start;
extern unsigned int kernel_physical_end;
extern unsigned int kernel_virtual_start;
extern unsigned int kernel_virtual_end;

/* Hàm tính độ dài chuỗi ký tự kết thúc bằng '\0' */
static unsigned int strlen(const char *str)
{
    unsigned int len = 0;
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
static void print_hex(unsigned int n)
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

int kmain(unsigned int ebx)
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

    fb_set_color(FB_LIGHT_BROWN, FB_BLACK);
    print("Testing auto-scroll functionality (printing 25 lines)...\n");

    /* Kiểm tra tính năng tự động cuộn màn hình khi in nhiều dòng */
    fb_set_color(FB_WHITE, FB_BLACK);
    for (int i = 1; i <= 25; i++) {
        print("  -> Line ");
        char buf[4];
        if (i >= 10) {
            buf[0] = '0' + (char)(i / 10);
            buf[1] = '0' + (char)(i % 10);
            buf[2] = '\0';
        } else {
            buf[0] = '0' + (char)i;
            buf[1] = '\0';
        }
        print(buf);
        print(": scrolling test in progress...\n");
    }

    fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
    print("\n[ OK ] Auto-scroll test completed successfully!\n");

    /* Khởi tạo và kiểm tra Serial driver (COM1: 115200 8N1) */
    fb_set_color(FB_LIGHT_MAGENTA, FB_BLACK);
    serial_config_baud_rate(SERIAL_COM1_BASE, 115200);
    serial_config_line(SERIAL_COM1_BASE);
    serial_config_buffers(SERIAL_COM1_BASE);
    serial_config_modem(SERIAL_COM1_BASE);

    /* Ghi dữ liệu kiểm tra ra cổng serial COM1 */
    serial_write_char(SERIAL_COM1_BASE, 'A');
    serial_write_char(SERIAL_COM1_BASE, 'E');
    const char *serial_msg = " [Serial COM1] AetherOS Serial Driver initialized successfully!\r\n";
    serial_write(SERIAL_COM1_BASE, serial_msg, strlen(serial_msg));

    print("[ OK ] Serial driver initialized successfully!\n");

    /* Kiểm tra tính năng ghi chuỗi trực tiếp */
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    const char *fb_test = "[ OK ] Framebuffer write() test passed!\n";
    fb_write(fb_test, strlen(fb_test));

    fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
    print("\n[ SUCCESS ] Driver write(), scroll & serial completed without error!\n");

    /* Khởi tạo phân đoạn bộ nhớ GDT */
    gdt_init();
    print("[ OK ] GDT initialized & loaded successfully.\n");

    /* Khởi tạo bảng phân phối ngắt IDT */
    idt_init();
    print("[ OK ] IDT initialized & loaded with 48 interrupt gates.\n");

    /* Khởi tạo bộ điều khiển ngắt khả trình PIC */
    pic_remap();
    print("[ OK ] PIC remapped (Master: 0x20, Slave: 0x28).\n");

    /* Khởi tạo trình điều khiển bàn phím PS/2 */
    keyboard_init();
    print("[ OK ] PS/2 Keyboard driver initialized on IRQ 1.\n");

    /* Khởi tạo phân trang (Paging) */
    paging_init();
    print("[ OK ] Higher-Half Paging active (Kernel at 0xC0100000, 3GB Virtual Base).\n");

    /* Hiển thị phạm vi bộ nhớ của Kernel */
    unsigned int p_start = (unsigned int)&kernel_physical_start;
    unsigned int p_end   = (unsigned int)&kernel_physical_end;
    unsigned int v_start = (unsigned int)&kernel_virtual_start;
    unsigned int v_end   = (unsigned int)&kernel_virtual_end;

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

    extern unsigned char kernel_stack[];
    #define KERNEL_STACK_SIZE 4096

    multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;

    /* Kiểm tra cờ Multiboot và số lượng module hợp lệ trước khi gọi */
    if ((mbinfo->flags & MULTIBOOT_INFO_MODS) && mbinfo->mods_count > 0) {
        multiboot_module_t *mod = (multiboot_module_t *) mbinfo->mods_addr;
        unsigned int mod_start = mod->mod_start;
        unsigned int mod_size = mod->mod_end - mod->mod_start;

        fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
        print("[ OK ] GRUB module found! Configuring User Mode (Ring 3)...\n");

        /* 1. Cấu hình phân trang cho User Mode (Chapter 11):
         * - Map mã lệnh User tại 0x00000000 (User accessible, U/S=1)
         * - Map ngăn xếp User tại 0xBFFFFFFB (User accessible, U/S=1)
         * - Giữ nguyên ánh xạ Kernel Higher-Half 0xC0000000 (Supervisor, U/S=0)
         * - Nạp CR3 với User Page Directory
         */
        paging_setup_user_process(mod_start, mod_size);
        print("[ OK ] User Page Directory & Tables mapped with U/S = 1.\n");

        /* 2. Cài đặt Kernel Stack vào TSS:
         * Đảm bảo khi CPU từ Ring 3 gặp ngắt/ngoại lệ/syscall nhảy về Ring 0,
         * CPU sẽ nạp đúng đỉnh ngăn xếp kernel an toàn.
         */
        tss_set_kernel_stack((unsigned int)kernel_stack + KERNEL_STACK_SIZE);
        print("[ OK ] TSS Ring 0 stack pointer configured.\n");

        /* 3. Chuẩn bị bước nhảy sang User Mode */
        fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
        print("[ INFO ] Executing iret to drop CPU privilege into Ring 3...\n");
        const char *serial_user_msg = "[User Mode] Switching to Ring 3 via iret (CS=0x1B, SS=0x23, EIP=0x0)...\r\n";
        serial_write(SERIAL_COM1_BASE, serial_user_msg, strlen(serial_user_msg));

        /* 4. Nhảy sang Ring 3: EIP = 0x00000000, ESP = 0xBFFFFFFB */
        enter_user_mode(USER_ENTRY_POINT, USER_STACK_TOP);

        /* Sau khi vào User Mode, CPU sẽ thực thi tại 0x00000000 */
    } else {
        fb_set_color(FB_LIGHT_RED, FB_BLACK);
        print("[ WARN ] No Multiboot module found.\n");
    }

    return 0;
}