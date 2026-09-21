#ifndef SERIAL_H
#define SERIAL_H
#define SERIAL_COM1_BASE                0x3F8 /* Cổng base của COM1 */ 

void serial_config_baud_rate(unsigned int base, unsigned int baud_rate);
void serial_config_line(unsigned short com);
int serial_is_transmit_fifo_empty(unsigned int com);

#endif