#include "gdt.h"

#define GDT_ENTRIES 3

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdtr;

/* Hàm Assembly load_gdt trong boot/loader.s */
extern void load_gdt(struct gdt_ptr *ptr);

static void gdt_set_gate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_init(void)
{
    gdtr.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gdtr.base = (unsigned int)&gdt;

    /* Mục 0: Null descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Mục 1: Kernel Code Segment (offset 0x08): Base 0, Limit 4GB, RX, Ring 0, 4KB granularity */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Mục 2: Kernel Data Segment (offset 0x10): Base 0, Limit 4GB, RW, Ring 0, 4KB granularity */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    load_gdt(&gdtr);
}
