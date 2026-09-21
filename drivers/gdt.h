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

void gdt_init(void);

#endif /* GDT_H */
