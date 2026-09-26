#include "arch/x86/gdt.h"

#define GDT_ENTRIES 6

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdtr;
static struct tss_entry tss;

/* Assembly routines declared in arch/x86/boot/loader.s */
extern void load_gdt(struct gdt_ptr *ptr);
extern void load_tss(uint16_t tss_selector);

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access = access;
}

static void write_tss(int num, uint16_t ss0, uint32_t esp0)
{
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss) - 1;

    /* Install TSS descriptor into GDT: Type 0x89 (32-bit available TSS, DPL=0, Present=1) */
    gdt_set_gate(num, base, limit, 0x89, 0x00);

    /* Clear the entire TSS structure */
    for (uint32_t i = 0; i < sizeof(tss); i++) {
        ((uint8_t *)&tss)[i] = 0;
    }

    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.cs = SEGMENT_KERNEL_CS | 0x3;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = SEGMENT_KERNEL_DS | 0x3;
    tss.iomap_base = sizeof(struct tss_entry);
}

void tss_set_kernel_stack(uint32_t stack)
{
    tss.esp0 = stack;
}

void gdt_init(void)
{
    gdtr.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gdtr.base = (uint32_t)&gdt;

    /* Entry 0: Mandatory null descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Entry 1: Kernel Code Segment (0x08): Base 0, Limit 4GB, RX, Ring 0, 4KB granularity */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Entry 2: Kernel Data Segment (0x10): Base 0, Limit 4GB, RW, Ring 0, 4KB granularity */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Entry 3: User Code Segment (0x18): Base 0, Limit 4GB, RX, Ring 3 (DPL=3), 4KB granularity */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    /* Entry 4: User Data Segment (0x20): Base 0, Limit 4GB, RW, Ring 3 (DPL=3), 4KB granularity */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    /* Entry 5: Task State Segment (0x28): Required for Ring 3 -> Ring 0 stack privilege transition */
    write_tss(5, SEGMENT_KERNEL_DS, 0);

    /* Load GDTR */
    load_gdt(&gdtr);

    /* Load Task Register (TR) */
    load_tss(SEGMENT_TSS);
}
