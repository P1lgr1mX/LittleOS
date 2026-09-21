#include "framebuffer.h"

/* Hàm tính độ dài chuỗi ký tự kết thúc bằng '\0' */
static unsigned int strlen(const char *str)
{
    unsigned int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/* Hàm phụ trợ in chuỗi ra màn hình dùng write() */
static void print(char *str)
{
    write(str, strlen(str));
}

int kmain(void)
{
    /* Xóa sạch màn hình và đưa con trỏ về (0, 0) */
    fb_clear();

    /* In tiêu đề với màu sắc sinh động */
    fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
    print("=================================================================\n");
    print("                 Welcome to AetherOS Kernel!                     \n");
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
            buf[0] = '0' + (i / 10);
            buf[1] = '0' + (i % 10);
            buf[2] = '\0';
        } else {
            buf[0] = '0' + i;
            buf[1] = '\0';
        }
        print(buf);
        print(": scrolling test in progress...\n");
    }

    fb_set_color(FB_LIGHT_MAGENTA, FB_BLACK);
    print("\n[ SUCCESS ] Driver write() & scroll completed without error!\n");

    return 0;
}