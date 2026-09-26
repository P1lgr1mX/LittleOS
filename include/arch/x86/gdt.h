#ifndef ARCH_X86_GDT_H
#define ARCH_X86_GDT_H

#include "types.h"

/* Standard x86 8-byte Segment Descriptor entry in GDT */
struct gdt_entry {
    uint16_t limit_low;       /* Segment limit bits 0..15 */
    uint16_t base_low;        /* Base address bits 0..15 */
    uint8_t  base_middle;     /* Base address bits 16..23 */
    uint8_t  access;          /* Access byte (Type, S, DPL, P flags) */
    uint8_t  granularity;     /* Granularity and limit bits 16..19 */
    uint8_t  base_high;       /* Base address bits 24..31 */
} __attribute__((packed));

/* GDTR pointer passed to the lgdt assembly instruction */
struct gdt_ptr {
    uint16_t limit;           /* Size of the GDT in bytes minus 1 */
    uint32_t base;            /* Linear base address of the GDT */
} __attribute__((packed));

/* Standard IA-32 Task State Segment (TSS) structure (104 bytes) */
struct tss_entry {
    uint32_t prev_tss;   /* Previous TSS backlink (for hardware task switching) */
    uint32_t esp0;       /* Ring 0 stack pointer loaded upon privilege transition */
    uint32_t ss0;        /* Ring 0 stack segment selector (0x10 - Kernel Data) */
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

/* GDT segment selector constants */
#define SEGMENT_KERNEL_CS 0x08
#define SEGMENT_KERNEL_DS 0x10
#define SEGMENT_USER_CS   0x18
#define SEGMENT_USER_DS   0x20
#define SEGMENT_TSS       0x28

#define USER_CS_SELECTOR  (SEGMENT_USER_CS | 0x3) /* 0x1B (RPL=3) */
#define USER_DS_SELECTOR  (SEGMENT_USER_DS | 0x3) /* 0x23 (RPL=3) */

void gdt_init(void);
void tss_set_kernel_stack(uint32_t stack);

#endif /* ARCH_X86_GDT_H */
