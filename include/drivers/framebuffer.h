#ifndef DRIVERS_FRAMEBUFFER_H
#define DRIVERS_FRAMEBUFFER_H

#include "types.h"

/* Standard VGA Text Mode 16-color palette */
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

/* VGA text-mode console dimensions */
#define FB_NUM_COLS      80
#define FB_NUM_ROWS      25

/* Low-level framebuffer character cell and hardware cursor routines */
void fb_write_cell(uint32_t offset, char c, uint8_t fg, uint8_t bg);
void fb_move_cursor(uint16_t pos);

/* Framebuffer console driver interface */
void fb_clear(void);
void fb_set_color(uint8_t fg, uint8_t bg);
int write(const char *buf, uint32_t len);
int fb_write(const char *buf, uint32_t len);

#endif /* DRIVERS_FRAMEBUFFER_H */
