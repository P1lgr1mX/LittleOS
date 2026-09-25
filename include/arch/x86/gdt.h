#ifndef ARCH_X86_GDT_H
#define ARCH_X86_GDT_H

#include "types.h"

/* Cấu trúc một mục Segment Descriptor trong GDT (8 bytes) */
struct gdt_entry {
    uint16_t limit_low;       /* 16 bit thấp của limit */
    uint16_t base_low;        /* 16 bit thấp của base */
    uint8_t  base_middle;     /* 8 bit tiếp theo của base */
    uint8_t  access;          /* Byte truy cập (Access byte) */
    uint8_t  granularity;     /* Byte độ phân giải và cờ (Granularity) */
    uint8_t  base_high;       /* 8 bit cao của base */
} __attribute__((packed));

/* Con trỏ GDTR truyền vào hàm load_gdt */
struct gdt_ptr {
    uint16_t limit;           /* Kích thước GDT - 1 */
    uint32_t base;            /* Địa chỉ cơ sở của bảng GDT */
} __attribute__((packed));

/* Cấu trúc Task State Segment (TSS) chuẩn x86 32-bit (104 bytes) */
struct tss_entry {
    uint32_t prev_tss;   /* TSS trước đó (nếu dùng task switching bằng phần cứng) */
    uint32_t esp0;       /* Con trỏ ngăn xếp Ring 0 (Kernel stack) khi chuyển đặc quyền */
    uint32_t ss0;        /* Phân đoạn ngăn xếp Ring 0 (0x10 - Kernel Data) */
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

/* Các hằng số Segment Selector của GDT */
#define SEGMENT_KERNEL_CS 0x08
#define SEGMENT_KERNEL_DS 0x10
#define SEGMENT_USER_CS   0x18
#define SEGMENT_USER_DS   0x20
#define SEGMENT_TSS       0x28

#define USER_CS_SELECTOR  (SEGMENT_USER_CS | 0x3) /* 0x1B */
#define USER_DS_SELECTOR  (SEGMENT_USER_DS | 0x3) /* 0x23 */

void gdt_init(void);
void tss_set_kernel_stack(uint32_t stack);

#endif /* ARCH_X86_GDT_H */
