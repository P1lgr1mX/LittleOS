#include "framebuffer.h"
#include "io.h"

/* Các cổng I/O điều khiển Framebuffer của VGA */
#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5

/* Các mã lệnh gửi tới cổng điều khiển */
#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

/* Địa chỉ bắt đầu của bộ đệm VGA text mode (0x000B8000) */
static char *fb = (char *) 0x000B8000;

/* Vị trí con trỏ hiện tại trên màn hình (0 -> 1999) */
static unsigned short cursor_pos = 0;

/* Màu mặc định: chữ trắng, nền đen */
static unsigned char current_fg = FB_WHITE;
static unsigned char current_bg = FB_BLACK;

/**
 * fb_write_cell:
 * Ghi một ký tự kèm màu chữ và màu nền vào vị trí byte i trong framebuffer.
 * Mỗi cell gồm 2 byte:
 *   - byte i: ký tự ASCII
 *   - byte i + 1: thuộc tính màu ((bg << 4) | fg)
 *
 * @param i   Vị trí byte bắt đầu (0, 2, 4, ..., 3998)
 * @param c   Ký tự ASCII
 * @param fg  Màu tiền cảnh (chữ) - 4 bit thấp (0-3)
 * @param bg  Màu hậu cảnh (nền)  - 4 bit cao (4-7)
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    fb[i] = c;
    fb[i + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

/**
 * fb_move_cursor:
 * Di chuyển con trỏ phần cứng trên màn hình.
 *
 * @param pos Vị trí ô (0 -> 1999)
 */
void fb_move_cursor(unsigned short pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    (unsigned char)((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    (unsigned char)(pos & 0x00FF));
}

/**
 * fb_scroll:
 * Tự động cuộn màn hình lên 1 dòng khi con trỏ chạm hoặc vượt quá dòng cuối.
 * Dòng đầu tiên sẽ bị đẩy ra ngoài, các dòng sau dịch lên trên 1 dòng,
 * và dòng cuối cùng được dọn sạch thành khoảng trắng.
 */
static void fb_scroll(void)
{
    /* Dịch chuyển từ dòng 1..24 lên dòng 0..23 (24 dòng * 80 cột * 2 byte = 3840 bytes) */
    unsigned int move_bytes = (FB_NUM_ROWS - 1) * FB_NUM_COLS * 2;
    for (unsigned int i = 0; i < move_bytes; i++) {
        fb[i] = fb[i + FB_NUM_COLS * 2];
    }

    /* Xóa dòng cuối cùng (dòng thứ 25, index 24) bằng ký tự khoảng trắng */
    unsigned int last_row_start = (FB_NUM_ROWS - 1) * FB_NUM_COLS * 2;
    unsigned int total_bytes = FB_NUM_ROWS * FB_NUM_COLS * 2;
    for (unsigned int i = last_row_start; i < total_bytes; i += 2) {
        fb[i] = ' ';
        fb[i + 1] = ((current_bg & 0x0F) << 4) | (current_fg & 0x0F);
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
    unsigned int total_bytes = FB_NUM_ROWS * FB_NUM_COLS * 2;
    for (unsigned int i = 0; i < total_bytes; i += 2) {
        fb[i] = ' ';
        fb[i + 1] = ((current_bg & 0x0F) << 4) | (current_fg & 0x0F);
    }
    cursor_pos = 0;
    fb_move_cursor(cursor_pos);
}

/**
 * fb_set_color:
 * Thiết lập màu chữ và màu nền cho các ký tự in tiếp theo.
 */
void fb_set_color(unsigned char fg, unsigned char bg)
{
    current_fg = fg;
    current_bg = bg;
}

/**
 * fb_put_char:
 * Ghi 1 ký tự vào vị trí con trỏ hiện tại, xử lý các ký tự điều khiển
 * (\n, \r, \t, \b) và tự động cuộn màn hình nếu vượt quá giới hạn.
 */
static void fb_put_char(char c)
{
    if (c == '\n') {
        /* Xuống dòng mới: chuyển con trỏ tới đầu dòng tiếp theo */
        cursor_pos = (cursor_pos / FB_NUM_COLS + 1) * FB_NUM_COLS;
    } else if (c == '\r') {
        /* Về đầu dòng hiện tại */
        cursor_pos = (cursor_pos / FB_NUM_COLS) * FB_NUM_COLS;
    } else if (c == '\t') {
        /* Tab: căn theo bước 8 cột */
        cursor_pos = (cursor_pos + 8) & ~(8 - 1);
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

    /* Nếu vị trí con trỏ vượt quá ô cuối cùng của màn hình (>= 2000), cuộn màn hình */
    if (cursor_pos >= FB_NUM_COLS * FB_NUM_ROWS) {
        fb_scroll();
    }

    /* Đồng bộ con trỏ phần cứng với vị trí con trỏ logic */
    fb_move_cursor(cursor_pos);
}

/**
 * write:
 * Giao diện chính của Trình điều khiển (The Driver).
 * Ghi nội dung từ bộ đệm buf có độ dài len lên màn hình.
 * Tự động di chuyển con trỏ sau mỗi ký tự và cuộn màn hình khi cần.
 *
 * @param buf Bộ đệm chứa các ký tự cần ghi
 * @param len Số lượng ký tự cần ghi
 * @return Số lượng ký tự đã ghi thành công
 */
/**
 * fb_write:
 * Ghi một chuỗi ký tự độ dài `len` ra framebuffer.
 */
int fb_write(const char *buf, unsigned int len)
{
    for (unsigned int i = 0; i < len; i++) {
        fb_put_char(buf[i]);
    }
    return (int) len;
}