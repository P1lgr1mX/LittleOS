#ifndef ARCH_X86_IDT_H
#define ARCH_X86_IDT_H

#include "types.h"

/* Cấu trúc một mục trong IDT (8 bytes) */
struct idt_entry {
    uint16_t offset_lowerbits; /* 16 bit thấp của con trỏ hàm ISR */
    uint16_t selector;         /* Code Segment Selector (0x08) */
    uint8_t  zero;             /* Không dùng, luôn bằng 0 */
    uint8_t  type_attr;        /* Cờ thuộc tính (0x8E: 32-bit Interrupt Gate, Ring 0) */
    uint16_t offset_higherbits;/* 16 bit cao của con trỏ hàm ISR */
} __attribute__((packed));

/* Cấu trúc IDTR nạp vào CPU qua lệnh lidt */
struct idt_ptr {
    uint16_t limit;            /* Kích thước IDT - 1 */
    uint32_t base;             /* Địa chỉ cơ sở của bảng IDT */
} __attribute__((packed));

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_init(void);

#endif /* ARCH_X86_IDT_H */
