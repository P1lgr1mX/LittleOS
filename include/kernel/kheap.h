#ifndef KERNEL_KHEAP_H
#define KERNEL_KHEAP_H

#include "types.h"

/* Kích thước một block trong bitmap heap (32 bytes) */
#define KHEAP_BLOCK_SIZE     32

/* Kích thước toàn bộ vùng heap dành riêng cho kernel (256 KB) */
#define KHEAP_SIZE           (256 * 1024)

/* Số lượng block tối đa trong heap */
#define KHEAP_TOTAL_BLOCKS   (KHEAP_SIZE / KHEAP_BLOCK_SIZE)

/* Kích thước bitmap tính bằng byte (mỗi bit đại diện cho 1 block) */
#define KHEAP_BITMAP_SIZE    (KHEAP_TOTAL_BLOCKS / 8)

/* Magic number kiểm tra tính toàn vẹn của khối cấp phát */
#define KHEAP_MAGIC          0x1EA9BEEF

/*
 * Cấu trúc Header nằm ngay trước mỗi vùng nhớ được cấp phát bởi kmalloc
 */
typedef struct kheap_header {
    uint32_t magic;         /* KHEAP_MAGIC dùng để kiểm tra bộ nhớ hợp lệ */
    uint32_t start_block;   /* Vị trí index block bắt đầu trong bitmap */
    uint32_t num_blocks;    /* Số lượng block đã cấp phát liên tiếp */
    size_t   size;          /* Kích thước dữ liệu (payload) yêu cầu ban đầu */
} __attribute__((packed)) kheap_header_t;

/* Khởi tạo Kernel Bitmap Heap */
void kheap_init(void);

/* Cấp phát bộ nhớ động trong kernel */
void *kmalloc(size_t size);

/* Cấp phát và xóa trắng vùng nhớ về 0 */
void *kcalloc(size_t num, size_t size);

/* Thay đổi kích thước khối bộ nhớ đã cấp phát */
void *krealloc(void *ptr, size_t new_size);

/* Giải phóng khối bộ nhớ đã cấp phát */
void kfree(void *ptr);

/* Các hàm thống kê bộ nhớ Heap */
size_t kheap_get_total_memory(void);
size_t kheap_get_used_memory(void);
size_t kheap_get_free_memory(void);
void kheap_print_stats(void);

#endif /* KERNEL_KHEAP_H */
