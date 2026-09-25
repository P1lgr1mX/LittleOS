#ifndef ARCH_X86_IO_H
#define ARCH_X86_IO_H

#include "types.h"

/**
 * outb:
 * Gửi 1 byte dữ liệu tới cổng I/O chỉ định. Được định nghĩa trong arch/x86/cpu/io.s
 */
void outb(uint16_t port, uint8_t value);

/**
 * inb:
 * Đọc 1 byte dữ liệu từ cổng I/O chỉ định. Được định nghĩa trong arch/x86/cpu/io.s
 */
uint8_t inb(uint16_t port);

/**
 * enable_interrupts:
 * Bật cờ ngắt CPU (sti).
 */
void enable_interrupts(void);

/**
 * disable_interrupts:
 * Tắt cờ ngắt CPU (cli).
 */
void disable_interrupts(void);

#endif /* ARCH_X86_IO_H */
