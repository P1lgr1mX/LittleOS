#ifndef DRIVERS_SERIAL_H
#define DRIVERS_SERIAL_H

#include "types.h"

#define SERIAL_COM1_BASE                0x3F8 /* Base I/O port address for COM1 UART */ 

void serial_init(void);
void serial_config_baud_rate(uint32_t base, uint32_t baud_rate);
void serial_config_line(uint16_t com);
void serial_config_buffers(uint16_t com);
void serial_config_modem(uint16_t com);
int serial_is_transmit_fifo_empty(uint32_t com);
int serial_write(uint32_t com, const char *buf, uint32_t len);
int serial_write_char(uint32_t com, char c);
int serial_receive(uint32_t com);
int serial_read_char(uint32_t com);

#endif /* DRIVERS_SERIAL_H */
