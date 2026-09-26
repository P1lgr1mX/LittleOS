#ifndef DRIVERS_KEYBOARD_H
#define DRIVERS_KEYBOARD_H

#include "types.h"

#define KBD_DATA_PORT    0x60
#define KBD_STATUS_PORT  0x64

/* Capacity of the circular keyboard input buffer */
#define KBD_BUFFER_SIZE  256

uint8_t read_scan_code(void);
void keyboard_init(void);

/* Keyboard circular buffer management routines */
void keyboard_put_char(char c);
int  keyboard_has_char(void);
int  keyboard_getchar(void);
char keyboard_read_char(void);

#endif /* DRIVERS_KEYBOARD_H */
