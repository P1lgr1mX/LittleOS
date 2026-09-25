#ifndef DRIVERS_KEYBOARD_H
#define DRIVERS_KEYBOARD_H

#include "types.h"

#define KBD_DATA_PORT    0x60
#define KBD_STATUS_PORT  0x64

uint8_t read_scan_code(void);
void keyboard_init(void);

#endif /* DRIVERS_KEYBOARD_H */
