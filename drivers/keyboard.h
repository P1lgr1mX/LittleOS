#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KBD_DATA_PORT    0x60
#define KBD_STATUS_PORT  0x64

unsigned char read_scan_code(void);
void keyboard_init(void);

#endif /* KEYBOARD_H */
