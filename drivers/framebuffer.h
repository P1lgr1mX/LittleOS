#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "io.h"

/* Các màu sắc tiêu chuẩn của VGA Text Mode (16 màu) */
#define FB_BLACK         0
#define FB_BLUE          1
#define FB_GREEN         2
#define FB_CYAN          3
#define FB_RED           4
#define FB_MAGENTA       5
#define FB_BROWN         6
#define FB_LIGHT_GREY    7
#define FB_DARK_GREY     8
#define FB_LIGHT_BLUE    9
#define FB_LIGHT_GREEN   10
#define FB_LIGHT_CYAN    11
#define FB_LIGHT_RED     12
#define FB_LIGHT_MAGENTA 13
#define FB_LIGHT_BROWN   14
#define FB_WHITE         15

/* Kích thước màn hình văn bản VGA */
#define FB_NUM_COLS      80
#define FB_NUM_ROWS      25

/* Giao diện tương tác tầng thấp với Framebuffer */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);
void fb_move_cursor(unsigned short pos);

/* Giao diện Trình điều khiển (The Driver) */
void fb_clear(void);
void fb_set_color(unsigned char fg, unsigned char bg);
int write(const char *buf, unsigned int len);
int fb_write(const char *buf, unsigned int len);

#endif /* FRAMEBUFFER_H */