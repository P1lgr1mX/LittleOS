#include "arch/x86/idt.h"

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtr;

/* Assembly routine declared in arch/x86/cpu/idt.s */
extern void load_idt(struct idt_ptr *ptr);

/* Array of 48 ISR stubs defined in arch/x86/cpu/isr.s */
extern void *isr_stub_table[48];

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt[num].offset_lowerbits = (uint16_t)(base & 0xFFFF);
    idt[num].offset_higherbits = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}

void idt_init(void)
{
    idtr.limit = (uint16_t)(sizeof(struct idt_entry) * IDT_ENTRIES - 1);
    idtr.base = (uint32_t)&idt;

    /* Clear all 256 IDT descriptors */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate((uint8_t)i, 0, 0, 0);
    }

    /* Install interrupt gates for vectors 0..47 (32 CPU exceptions and 16 hardware IRQs) */
    for (uint8_t i = 0; i < 48; i++) {
        idt_set_gate(i, (uint32_t)isr_stub_table[i], 0x08, 0x8E);
    }

    /* Load IDTR register via assembly lidt */
    load_idt(&idtr);
}
