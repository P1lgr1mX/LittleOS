#include "idt.h"

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtr;

/* Khai báo hàm Assembly load_idt trong drivers/idt.s */
extern void load_idt(struct idt_ptr *ptr);

/* Mảng địa chỉ 48 ISR stubs được định nghĩa trong drivers/isr.s */
extern void *isr_stub_table[48];

void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags)
{   //int 0x80 intel
    idt[num].offset_lowerbits = (unsigned short)(base & 0xFFFF);
    idt[num].offset_higherbits = (unsigned short)((base >> 16) & 0xFFFF);
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}

void idt_init(void)
{
    idtr.limit = (unsigned short)(sizeof(struct idt_entry) * IDT_ENTRIES - 1);
    idtr.base = (unsigned int)&idt;

    /* Xóa sạch 256 mục trong IDT */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate((unsigned char)i, 0, 0, 0);
    }

    /* Thiết lập các cổng ngắt từ bảng isr_stub_table (0 đến 47: bao gồm 32 CPU exceptions và 16 IRQs) */
    for (unsigned char i = 0; i < 48; i++) {
        idt_set_gate(i, (unsigned int)isr_stub_table[i], 0x08, 0x8E);
    }

    /* Nạp IDTR vào thanh ghi CPU qua lệnh lidt */
    load_idt(&idtr);

}


