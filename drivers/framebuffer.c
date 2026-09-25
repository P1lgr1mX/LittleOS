#include "drivers/framebuffer.h"
#include "arch/x86/io.h"

/* Các cổng I/O điều khiển Framebuffer của VGA */
#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5

/* Các mã lệnh gửi tới cổng điều khiển */
#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

/* 
 * Địa chỉ bắt đầu của bộ đệm VGA text mode trong không gian Higher-Half (0xC00B8000).
 * Địa chỉ vật lý là 0x000B8000. Do bảng trang ánh xạ 3GB (0xC0000000) vào 0x00000000 vật lý,
 * địa chỉ ảo truy cập qua Paging là: 0xC0000000 + 0x000B8000 = 0xC00B8000.
 */
static char *fb = (char *) 0xC00B8000;

/* Vị trí con trỏ hiện tại trên màn hình (0 -> 1999) */
static uint16_t cursor_pos = 0;

/* Màu mặc định: chữ trắng, nền đen */
static uint8_t current_fg = FB_WHITE;
static uint8_t current_bg = FB_BLACK;

/**
 * fb_write_cell:
 * Ghi một ký tự kèm màu chữ và màu nền vào vị trí byte i trong framebuffer.
 * Mỗi cell gồm 2 byte:
 *   - byte i: ký tự ASCII
 *   - byte i + 1: thuộc tính màu ((bg << 4) | fg)
 */
void fb_write_cell(uint32_t i, char c, uint8_t fg, uint8_t bg)
{
    fb[i] = c;
    fb[i + 1] = (char)(((bg & 0x0F) << 4) | (fg & 0x0F));
}

/**
 * fb_move_cursor:
 * Di chuyển con trỏ phần cứng trên màn hình.
 */
void fb_move_cursor(uint16_t pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    (uint8_t)((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    (uint8_t)(pos & 0x00FF));
}

/**
 * fb_scroll:
 * Tự động cuộn màn hình lên 1 dòng khi con trỏ chạm hoặc vượt quá dòng cuối.
 */
static void fb_scroll(void)
{
    /* Dịch chuyển từ dòng 1..24 lên dòng 0..23 */
    uint32_t move_bytes = (FB_NUM_ROWS - 1) * FB_NUM_COLS * 2;
    for (uint32_t i = 0; i < move_bytes; i++) {
        fb[i] = fb[i + FB_NUM_COLS * 2];
    }

    /* Xóa dòng cuối cùng (dòng thứ 25, index 24) bằng ký tự khoảng trắng */
    uint32_t last_row_start = (FB_NUM_ROWS - 1) * FB_NUM_COLS * 2;
    uint32_t total_bytes = FB_NUM_ROWS * FB_NUM_COLS * 2;
    for (uint32_t i = last_row_start; i < total_bytes; i += 2) {
        fb[i] = ' ';
        fb[i + 1] = (char)(((current_bg & 0x0F) << 4) | (current_fg & 0x0F));
    }

    /* Đặt con trỏ về đầu dòng cuối cùng */
    cursor_pos = (FB_NUM_ROWS - 1) * FB_NUM_COLS;
}

/**
 * fb_clear:
 * Xóa toàn bộ màn hình và đưa con trỏ về góc trên cùng bên trái (0, 0).
 */
void fb_clear(void)
{
    uint32_t total_bytes = FB_NUM_ROWS * FB_NUM_COLS * 2;
    for (uint32_t i = 0; i < total_bytes; i += 2) {
        fb[i] = ' ';
        fb[i + 1] = (char)(((current_bg & 0x0F) << 4) | (current_fg & 0x0F));
    }
    cursor_pos = 0;
    fb_move_cursor(cursor_pos);
}

/**
 * fb_set_color:
 * Thiết lập màu chữ và màu nền cho các ký tự in tiếp theo.
 */
void fb_set_color(uint8_t fg, uint8_t bg)
{
    current_fg = fg;
    current_bg = bg;
}

/**
 * fb_put_char:
 * Ghi 1 ký tự vào vị trí con trỏ hiện tại, xử lý các ký tự điều khiển
 */
static void fb_put_char(char c)
{
    if (c == '\n') {
        /* Xuống dòng mới: chuyển con trỏ tới đầu dòng tiếp theo */
        cursor_pos = (uint16_t)((cursor_pos / FB_NUM_COLS + 1) * FB_NUM_COLS);
    } else if (c == '\r') {
        /* Về đầu dòng hiện tại */
        cursor_pos = (uint16_t)((cursor_pos / FB_NUM_COLS) * FB_NUM_COLS);
    } else if (c == '\t') {
        /* Tab: căn theo bước 8 cột */
        cursor_pos = (uint16_t)((cursor_pos + 8) & ~(8 - 1));
    } else if (c == '\b') {
        /* Phím xóa lùi: lùi 1 ô và xóa ký tự */
        if (cursor_pos > 0) {
            cursor_pos--;
            fb_write_cell(cursor_pos * 2, ' ', current_fg, current_bg);
        }
    } else {
        /* Ký tự thông thường: ghi vào vị trí con trỏ hiện tại */
        fb_write_cell(cursor_pos * 2, c, current_fg, current_bg);
        cursor_pos++;
    }

    /* Nếu vị trí con trỏ vượt quá ô cuối cùng của màn hình, cuộn màn hình */
    if (cursor_pos >= FB_NUM_COLS * FB_NUM_ROWS) {
        fb_scroll();
    }

    /* Đồng bộ con trỏ phần cứng */
    fb_move_cursor(cursor_pos);
}

/**
 * fb_write:
 * Ghi một chuỗi ký tự độ dài `len` ra framebuffer.
 */
int fb_write(const char *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        fb_put_char(buf[i]);
    }
    return (int) len;
}

/**
 * write:
 * Bí danh tiện ích cho hàm fb_write
 */
int write(const char *buf, uint32_t len)
{
    return fb_write(buf, len);
}