#ifndef IDT_H
#define IDT_H

/* Cấu trúc một mục trong IDT (8 bytes) */
struct idt_entry {
    unsigned short offset_lowerbits; /* 16 bit thấp của con trỏ hàm ISR */
    unsigned short selector;         /* Code Segment Selector (0x08) */
    unsigned char  zero;             /* Không dùng, luôn bằng 0 */
    unsigned char  type_attr;        /* Cờ thuộc tính (0x8E: 32-bit Interrupt Gate, Ring 0) */
    unsigned short offset_higherbits;/* 16 bit cao của con trỏ hàm ISR */
} __attribute__((packed));

/* Cấu trúc IDTR nạp vào CPU qua lệnh lidt */
struct idt_ptr {
    unsigned short limit;            /* Kích thước IDT - 1 */
    unsigned int   base;             /* Địa chỉ cơ sở của bảng IDT */
} __attribute__((packed));

void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags);
void idt_init(void);

#endif /* IDT_H */
