#include "gdt.h"

#define GDT_ENTRIES 6

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdtr;
static struct tss_entry tss;

/* Các hàm Assembly trong boot/loader.s */
extern void load_gdt(struct gdt_ptr *ptr);
extern void load_tss(unsigned short tss_selector);

static void gdt_set_gate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran)
{
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access = access;
}

static void write_tss(int num, unsigned short ss0, unsigned int esp0)
{
    unsigned int base = (unsigned int)&tss;
    unsigned int limit = sizeof(tss) - 1;

    /* Cài đặt TSS Descriptor vào GDT: Type 0x89 (32-bit TSS khả dụng, DPL=0, Present=1) */
    gdt_set_gate(num, base, limit, 0x89, 0x00);

    /* Xóa sạch cấu trúc TSS */
    for (unsigned int i = 0; i < sizeof(tss); i++) {
        ((unsigned char *)&tss)[i] = 0;
    }

    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.cs = SEGMENT_KERNEL_CS | 0x3;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = SEGMENT_KERNEL_DS | 0x3;
    tss.iomap_base = sizeof(struct tss_entry);
}

void tss_set_kernel_stack(unsigned int stack)
{
    tss.esp0 = stack;
}

void gdt_init(void)
{
    gdtr.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gdtr.base = (unsigned int)&gdt;

    /* Mục 0: Null descriptor */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Mục 1: Kernel Code Segment (0x08): Base 0, Limit 4GB, RX, Ring 0, 4KB granularity */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Mục 2: Kernel Data Segment (0x10): Base 0, Limit 4GB, RW, Ring 0, 4KB granularity */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Mục 3: User Code Segment (0x18): Base 0, Limit 4GB, RX, Ring 3 (DPL=3), 4KB granularity */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    /* Mục 4: User Data Segment (0x20): Base 0, Limit 4GB, RW, Ring 3 (DPL=3), 4KB granularity */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    /* Mục 5: Task State Segment (TSS) (0x28): Cần thiết khi chuyển đặc quyền Ring 3 -> Ring 0 */
    write_tss(5, SEGMENT_KERNEL_DS, 0);

    /* Nạp GDTR */
    load_gdt(&gdtr);

    /* Nạp Task Register (TR) */
    load_tss(SEGMENT_TSS);
}
