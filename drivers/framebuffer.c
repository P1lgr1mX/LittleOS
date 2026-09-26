#include "drivers/framebuffer.h"
#include "arch/x86/io.h"

/* VGA CRT Controller I/O ports */
#define FB_COMMAND_PORT         0x3D4
#define FB_DATA_PORT            0x3D5

/* CRT Controller register indices for cursor location */
#define FB_HIGH_BYTE_COMMAND    14
#define FB_LOW_BYTE_COMMAND     15

/* 
 * Linear address of VGA text-mode memory in higher-half virtual space (0xC00B8000).
 * Physical address is 0x000B8000. With higher-half paging mapping physical 0x00000000
 * to virtual 0xC0000000, virtual address is: 0xC0000000 + 0x000B8000 = 0xC00B8000.
 */
static char *fb = (char *) 0xC00B8000;

/* Current cursor offset on screen (0 -> (FB_NUM_COLS * FB_NUM_ROWS - 1)) */
static uint16_t cursor_pos = 0;

/* Current active text attribute colors (default: white text on black background) */
static uint8_t current_fg = FB_WHITE;
static uint8_t current_bg = FB_BLACK;

/**
 * fb_write_cell:
 * Writes an ASCII character with foreground and background attributes to the
 * character cell at offset `offset` in the framebuffer.
 * Each character cell consists of 2 contiguous bytes:
 *   - byte 0 (offset): ASCII character code
 *   - byte 1 (offset + 1): attribute byte ((bg << 4) | fg)
 */
void fb_write_cell(uint32_t offset, char c, uint8_t fg, uint8_t bg)
{
    fb[offset] = c;
    fb[offset + 1] = (char)(((bg & 0x0F) << 4) | (fg & 0x0F));
}

/**
 * fb_move_cursor:
 * Updates the hardware cursor position via VGA CRT Controller registers.
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
 * Scrolls the console up by one line when the cursor exceeds screen bounds.
 */
static void fb_scroll(void)
{
    /* Shift rows 1..24 up to rows 0..23 */
    uint32_t move_bytes = (FB_NUM_ROWS - 1) * FB_NUM_COLS * 2;
    for (uint32_t i = 0; i < move_bytes; i++) {
        fb[i] = fb[i + FB_NUM_COLS * 2];
    }

    /* Clear the last line with blank space characters using active colors */
    uint32_t last_row_start = (FB_NUM_ROWS - 1) * FB_NUM_COLS * 2;
    uint32_t total_bytes = FB_NUM_ROWS * FB_NUM_COLS * 2;
    for (uint32_t i = last_row_start; i < total_bytes; i += 2) {
        fb[i] = ' ';
        fb[i + 1] = (char)(((current_bg & 0x0F) << 4) | (current_fg & 0x0F));
    }

    /* Position the cursor at the beginning of the last row */
    cursor_pos = (FB_NUM_ROWS - 1) * FB_NUM_COLS;
}

/**
 * fb_clear:
 * Clears the entire console display and resets the cursor to (0, 0).
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
 * Sets active foreground and background colors for subsequent character output.
 */
void fb_set_color(uint8_t fg, uint8_t bg)
{
    current_fg = fg;
    current_bg = bg;
}

/**
 * fb_put_char:
 * Writes a character at the current cursor position, handling control codes.
 */
static void fb_put_char(char c)
{
    if (c == '\n') {
        /* Line feed: advance cursor to the beginning of the next row */
        cursor_pos = (uint16_t)((cursor_pos / FB_NUM_COLS + 1) * FB_NUM_COLS);
    } else if (c == '\r') {
        /* Carriage return: move cursor to the start of the current row */
        cursor_pos = (uint16_t)((cursor_pos / FB_NUM_COLS) * FB_NUM_COLS);
    } else if (c == '\t') {
        /* Horizontal tab: align cursor to next 8-column tab stop */
        cursor_pos = (uint16_t)((cursor_pos + 8) & ~(8 - 1));
    } else if (c == '\b') {
        /* Backspace: move cursor back one cell and erase character */
        if (cursor_pos > 0) {
            cursor_pos--;
            fb_write_cell(cursor_pos * 2, ' ', current_fg, current_bg);
        }
    } else {
        /* Printable character: write to current cell */
        fb_write_cell(cursor_pos * 2, c, current_fg, current_bg);
        cursor_pos++;
    }

    /* Scroll if cursor reaches or exceeds display buffer bounds */
    if (cursor_pos >= FB_NUM_COLS * FB_NUM_ROWS) {
        fb_scroll();
    }

    /* Synchronize VGA hardware cursor */
    fb_move_cursor(cursor_pos);
}

/**
 * fb_write:
 * Outputs a character buffer of length `len` to the console framebuffer.
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
 * Standard output wrapper alias for fb_write.
 */
int write(const char *buf, uint32_t len)
{
    return fb_write(buf, len);
}