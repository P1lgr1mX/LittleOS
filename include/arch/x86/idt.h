#ifndef ARCH_X86_IDT_H
#define ARCH_X86_IDT_H

#include "types.h"

/* Standard x86 8-byte Interrupt Gate Descriptor in IDT */
struct idt_entry {
    uint16_t offset_lowerbits; /* Lower 16 bits of ISR function pointer */
    uint16_t selector;         /* Kernel Code Segment Selector (0x08) */
    uint8_t  zero;             /* Reserved, must be 0 */
    uint8_t  type_attr;        /* Type and attribute flags (0x8E: 32-bit Interrupt Gate, Ring 0) */
    uint16_t offset_higherbits;/* Upper 16 bits of ISR function pointer */
} __attribute__((packed));

/* IDTR pointer structure loaded via lidt instruction */
struct idt_ptr {
    uint16_t limit;            /* Size of the IDT in bytes minus 1 */
    uint32_t base;             /* Linear base address of the IDT */
} __attribute__((packed));

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_init(void);

#endif /* ARCH_X86_IDT_H */
