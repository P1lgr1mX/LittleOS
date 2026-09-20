#include "io.h"
#define SERIAL_COM1_BASE                0x3F8 /* Cổng base của COM1 */
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

#define SERIAL_LINE_ENABLED_DLAB        0x80


void serial_config_baud_rate(unsigned int base, unsigned int baud_rate){
    outb(SERIAL_LINE_COMMAND_PORT(base), SERIAL_LINE_ENABLED_DLAB);
    outb(SERIAL_DATA_PORT(base), baud_rate >> 8 & 0xFF);
    outb(SERIAL_DATA_PORT(base), baud_rate & 0xFF);
}