#include "drivers/serial.h"
#include "drivers/io.h"
#define SERIAL_COM1_BASE                0x3F8 /* Cổng base của COM1 */
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

#define SERIAL_LINE_ENABLED_DLAB        0x80


void serial_config_baud_rate(unsigned int base, unsigned int baud_rate)
{
    unsigned short divisor = (baud_rate >= 110) ? (115200 / baud_rate) : (baud_rate == 0 ? 1 : baud_rate);
    outb(SERIAL_LINE_COMMAND_PORT(base), SERIAL_LINE_ENABLED_DLAB);
    outb(SERIAL_DATA_PORT(base), (unsigned char)(divisor & 0xFF));
    outb(SERIAL_DATA_PORT(base) + 1, (unsigned char)((divisor >> 8) & 0xFF));
}

void serial_config_line(unsigned short com)
{
    /* 0x03: 8 bit dữ liệu, 1 stop bit, không kiểm tra chẵn lẻ (8N1), tắt DLAB */
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

void serial_config_buffers(unsigned short com)
{
    /* 0xC7: Bật FIFO, xóa buffer, ngưỡng ngắt 14 bytes */
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}

void serial_config_modem(unsigned short com)
{
    /* 0x03: Đặt RTS và DTR bằng 1 để báo sẵn sàng truyền nhận */
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
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

int serial_write(unsigned int com, const char *buf, unsigned int len)
{
    for (unsigned int i = 0; i < len; i++) {
        while (serial_is_transmit_fifo_empty(com) == 0);
        outb(SERIAL_DATA_PORT(com), buf[i]);
    }
    return (int) len;
}

int serial_write_char(unsigned int com, char c)
{
    while (serial_is_transmit_fifo_empty(com) == 0);
    outb(SERIAL_DATA_PORT(com), c);
    return 1;
}

int serial_receive(unsigned int com)
{
    /* Bit 0: Data Ready (1 khi có byte dữ liệu trong bộ đệm nhận) */
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 1;
}

int serial_read_char(unsigned int com)
{
    if (serial_receive(com)) {
        return inb(SERIAL_DATA_PORT(com));
    }
    return -1;
}