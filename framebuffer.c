#include "framebuffer.h"
#include "io.h" 

#define FB_COMMAND_PORT 0x3D4 
#define FB_DATA_PORT 0x3D5 
#define FB_HIGHT BYTE_COMMAND 14 
#define FB_LOW BYTE_COMMAND 15 

/**
 * fb_write_cell:
 * Ghi một ký tự kèm màu chữ và màu nền vào vị trí byte i trong framebuffer.
 *
 * @param i   Vị trí byte bắt đầu (0, 2, 4, ...)
 * @param c   Ký tự ASCII
 * @param fg  Màu tiền cảnh (chữ) - chiếm 4 bit thấp (0-3)
 * @param bg  Màu hậu cảnh (nền)  - chiếm 4 bit cao (4-7)
 */

 void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg){
    fb[i] = c; 
    fb[i + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F); 
 }

 void fb_move_cursor(unsigned short pos){ 
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    pos & 0x00FF);
}