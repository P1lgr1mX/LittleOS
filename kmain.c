#define FB_GREEN 2
#define FB_DARK_GREY 8

/* Khai báo con trỏ tới vùng nhớ Framebuffer */
char *fb = (char *) 0x000B8000;

/**
 * fb_write_cell:
 * Ghi một ký tự kèm màu chữ và màu nền vào vị trí byte i trong framebuffer.
 *
 * @param i   Vị trí byte bắt đầu (0, 2, 4, ...)
 * @param c   Ký tự ASCII
 * @param fg  Màu tiền cảnh (chữ) - chiếm 4 bit thấp (0-3)
 * @param bg  Màu hậu cảnh (nền)  - chiếm 4 bit cao (4-7)
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    fb[i] = c;
    fb[i + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

int kmain(void)
{
    /* Ghi chữ 'A' vào ô đầu tiên (hàng 0, cột 0) */
    fb_write_cell(0, 'A', FB_GREEN, FB_DARK_GREY);
    /* Ghi chữ 'B' vào ô đầu tiên (hàng 0, cột 1) */
    fb_write_cell(1, 'Z', FB_GREEN, FB_DARK_GREY);
    return 0;
}