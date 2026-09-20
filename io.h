#ifndef IO_H
#define IO_H

/**
 * outb:
 * Gửi dữ liệu được cung cấp tới cổng I/O chỉ định. Được định nghĩa trong io.s
 *
 * @param port  Cổng I/O cần gửi dữ liệu tới
 * @param value Dữ liệu (1 byte) cần gửi tới cổng I/O
 */
void outb(unsigned short port, unsigned char value);

#endif /* IO_H */