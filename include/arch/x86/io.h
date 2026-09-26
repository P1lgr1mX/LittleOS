#ifndef ARCH_X86_IO_H
#define ARCH_X86_IO_H

#include "types.h"

/**
 * outb:
 * Outputs an 8-bit byte value to the specified 16-bit I/O port.
 * Defined in arch/x86/cpu/io.s.
 */
void outb(uint16_t port, uint8_t value);

/**
 * inb:
 * Inputs an 8-bit byte value from the specified 16-bit I/O port.
 * Defined in arch/x86/cpu/io.s.
 */
uint8_t inb(uint16_t port);

/**
 * enable_interrupts:
 * Enables CPU hardware interrupts by executing the sti instruction.
 */
void enable_interrupts(void);

/**
 * disable_interrupts:
 * Disables CPU hardware interrupts by executing the cli instruction.
 */
void disable_interrupts(void);

#endif /* ARCH_X86_IO_H */
