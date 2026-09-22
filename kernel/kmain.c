#include "framebuffer.h"
#include "serial.h"
#include "io.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "multiboot.h"

typedef void (*call_module_t)(void);

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

int kmain(/* additional arguments */ unsigned int ebx)
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
    unsigned int address_of_module = mbinfo->mods_addr;

    /* Kiểm tra cờ Multiboot và số lượng module hợp lệ trước khi gọi */
    if ((mbinfo->flags & MULTIBOOT_INFO_MODS) && mbinfo->mods_count > 0) {
        multiboot_module_t *mod = (multiboot_module_t *) mbinfo->mods_addr;
        address_of_module = mod->mod_start;

        fb_set_color(FB_LIGHT_GREEN, FB_BLACK);
        print("[ OK ] GRUB module found! Executing module...\n");

        call_module_t start_program = (call_module_t) address_of_module;
        start_program();
        /* chúng ta sẽ không bao giờ tới được đây, trừ khi mã của mô-đun trả về */
    } else {
        fb_set_color(FB_LIGHT_RED, FB_BLACK);
        print("[ WARN ] No Multiboot module found.\n");
    }

    return 0;
}