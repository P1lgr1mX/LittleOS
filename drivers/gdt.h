#ifndef GDT_H
#define GDT_H

/* Cấu trúc một mục Segment Descriptor trong GDT (8 bytes) */
struct gdt_entry {
    unsigned short limit_low;       /* 16 bit thấp của limit */
    unsigned short base_low;        /* 16 bit thấp của base */
    unsigned char  base_middle;     /* 8 bit tiếp theo của base */
    unsigned char  access;          /* Byte truy cập (Access byte) */
    unsigned char  granularity;     /* Byte độ phân giải và cờ (Granularity) */
    unsigned char  base_high;       /* 8 bit cao của base */
} __attribute__((packed));

/* Con trỏ GDTR truyền vào hàm load_gdt */
struct gdt_ptr {
    unsigned short limit;           /* Kích thước GDT - 1 */
    unsigned int   base;            /* Địa chỉ cơ sở của bảng GDT */
} __attribute__((packed));

/* Cấu trúc Task State Segment (TSS) chuẩn x86 32-bit (104 bytes) */
struct tss_entry {
    unsigned int prev_tss;   /* TSS trước đó (nếu dùng task switching bằng phần cứng) */
    unsigned int esp0;       /* Con trỏ ngăn xếp Ring 0 (Kernel stack) khi chuyển đặc quyền */
    unsigned int ss0;        /* Phân đoạn ngăn xếp Ring 0 (0x10 - Kernel Data) */
    unsigned int esp1;
    unsigned int ss1;
    unsigned int esp2;
    unsigned int ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax;
    unsigned int ecx;
    unsigned int edx;
    unsigned int ebx;
    unsigned int esp;
    unsigned int ebp;
    unsigned int esi;
    unsigned int edi;
    unsigned int es;
    unsigned int cs;
    unsigned int ss;
    unsigned int ds;
    unsigned int fs;
    unsigned int gs;
    unsigned int ldt;
    unsigned short trap;
    unsigned short iomap_base;
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
void tss_set_kernel_stack(unsigned int stack);

#endif /* GDT_H */
