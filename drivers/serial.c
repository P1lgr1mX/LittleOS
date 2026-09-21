#include "drivers/serial.h"
#include "drivers/io.h"
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

void serial_config_line(unsigned short com){
    outb(SERIAL_LINE_COMMAND_PORT(SERIAL_COM1_BASE), com);
}

/** serial_is_transmit_fifo_empty:
 * Checks whether the transmit FIFO queue is empty or not for the given COM port.
 *
 * @param com The COM port
 * @return 0 if the transmit FIFO queue is not empty
 *         non-zero (0x20) if the transmit FIFO queue is empty
 */
int serial_is_transmit_fifo_empty(unsigned int com)
{
    /* 0x20 = 0010 0000: Bit 5 (Transmitter Holding Register Empty) */
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

