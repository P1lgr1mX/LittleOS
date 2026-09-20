#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include "io.h"

/* Khai báo con trỏ tới vùng nhớ Framebuffer */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);
void fb_move_cursor(unsigned short pos); 