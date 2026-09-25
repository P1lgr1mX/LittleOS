/*Sanemy - Nguyen Huy Quang x Gemini 3-8-flash 
    Day 25 / 9 / 2026 
    This file is part of the Sanemy - Nguyen Huy Quang x Gemini 3-8-flash 
    coursework for the course "Advanced Embedded Systems" at the
    HUST, Vietnam.
    GPL v3 License 
*/


#include "kernel/kheap.h"
#include "drivers/framebuffer.h"
#include "drivers/serial.h"

/* Vùng nhớ vật lý / ảo cấp cho heap kernel trong .bss (căn chỉnh 4KB) */
static uint8_t kheap_memory[KHEAP_SIZE] __attribute__((aligned(4096)));

/* Bảng Bitmap quản lý trạng thái từng block (0 = Free, 1 = Used) */
static uint8_t kheap_bitmap[KHEAP_BITMAP_SIZE];

/* Trạng thái khởi tạo của heap */
static int kheap_initialized = 0;

/* Các hàm phụ trợ bộ nhớ tầng thấp (thay thế libc trong kernel freestanding) */
static void *kheap_memset(void *dest, int val, size_t count)
{
    uint8_t *d = (uint8_t *)dest;
    for (size_t i = 0; i < count; i++) {
        d[i] = (uint8_t)val;
    }
    return dest;
}

static void *kheap_memcpy(void *dest, const void *src, size_t count)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

/* Thao tác bit trên bitmap */
static inline void bitmap_set(uint32_t bit)
{
    kheap_bitmap[bit / 8] |= (uint8_t)(1 << (bit % 8));
}

static inline void bitmap_clear(uint32_t bit)
{
    kheap_bitmap[bit / 8] &= (uint8_t)~(1 << (bit % 8));
}

static inline int bitmap_test(uint32_t bit)
{
    return (kheap_bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}

/**
 * kheap_init:
 * Khởi tạo vùng nhớ Bitmap Heap cho Kernel.
 * Đặt toàn bộ bitmap về 0 (tất cả các block đều tự do).
 */
void kheap_init(void)
{
    kheap_memset(kheap_bitmap, 0, sizeof(kheap_bitmap));
    kheap_memset(kheap_memory, 0, sizeof(kheap_memory));
    kheap_initialized = 1;
}

/**
 * kmalloc:
 * Cấp phát vùng nhớ động liên tiếp sử dụng thuật toán First-Fit trên Bitmap.
 *
 * @param size Số byte dữ liệu yêu cầu
 * @return Con trỏ tới vùng dữ liệu đã cấp phát hoặc NULL nếu hết bộ nhớ
 */
void *kmalloc(size_t size)
{
    if (!kheap_initialized || size == 0) {
        return NULL;
    }

    /* Kích thước cần thiết bao gồm Header quản lý + Kích thước yêu cầu */
    size_t total_needed = sizeof(kheap_header_t) + size;
    uint32_t blocks_needed = (uint32_t)((total_needed + KHEAP_BLOCK_SIZE - 1) / KHEAP_BLOCK_SIZE);

    if (blocks_needed > KHEAP_TOTAL_BLOCKS) {
        return NULL;
    }

    /* Tìm kiếm blocks_needed block liên tiếp còn trống (bit = 0) */
    uint32_t consecutive_free = 0;
    uint32_t start_block = 0;

    for (uint32_t i = 0; i < KHEAP_TOTAL_BLOCKS; i++) {
        if (!bitmap_test(i)) {
            if (consecutive_free == 0) {
                start_block = i;
            }
            consecutive_free++;

            if (consecutive_free == blocks_needed) {
                /* Đánh dấu các block đã sử dụng trong bitmap */
                for (uint32_t b = start_block; b < start_block + blocks_needed; b++) {
                    bitmap_set(b);
                }

                /* Điền thông tin vào header */
                kheap_header_t *header = (kheap_header_t *)&kheap_memory[start_block * KHEAP_BLOCK_SIZE];
                header->magic = KHEAP_MAGIC; //check buffer overflow here
                header->start_block = start_block;
                header->num_blocks = blocks_needed;
                header->size = size;

                /* Trả về địa chỉ ngay sau header để caller sử dụng */
                return (void *)((uint8_t *)header + sizeof(kheap_header_t));
            }
        } else {
            consecutive_free = 0;
        }
    }

    return NULL; /* Không tìm thấy đủ block trống liên tiếp */
}

/**
 * kcalloc:
 * Cấp phát bộ nhớ cho mảng num phần tử, mỗi phần tử kích thước size, và xóa về 0.
 */
void *kcalloc(size_t num, size_t size)
{
    size_t total_size = num * size;
    void *ptr = kmalloc(total_size);
    if (ptr) {
        kheap_memset(ptr, 0, total_size);
    }
    return ptr;
}

/**
 * krealloc:
 * Thay đổi kích thước khối bộ nhớ đã cấp phát.
 */
void *krealloc(void *ptr, size_t new_size)
{
    if (!ptr) {
        return kmalloc(new_size);
    }

    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    kheap_header_t *header = (kheap_header_t *)((uint8_t *)ptr - sizeof(kheap_header_t));
    if (header->magic != KHEAP_MAGIC) {
        return NULL;
    }

    /* Kiểm tra xem dung lượng các block hiện tại có đủ chứa new_size không */
    size_t current_capacity = (header->num_blocks * KHEAP_BLOCK_SIZE) - sizeof(kheap_header_t);
    if (new_size <= current_capacity) {
        header->size = new_size;
        return ptr;
    }

    /* Cấp phát vùng mới và sao chép dữ liệu cũ */
    void *new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL;
    }

    size_t copy_bytes = (header->size < new_size) ? header->size : new_size;
    kheap_memcpy(new_ptr, ptr, copy_bytes);
    kfree(ptr);

    return new_ptr;
}

/**
 * kfree:
 * Giải phóng khối bộ nhớ đã cấp phát qua kmalloc/kcalloc/krealloc.
 */
void kfree(void *ptr)
{
    if (!ptr || !kheap_initialized) {
        return;
    }

    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t heap_start = (uintptr_t)kheap_memory;
    uintptr_t heap_end = heap_start + KHEAP_SIZE;

    /* Đảm bảo con trỏ nằm hoàn toàn trong phạm vi heap */
    if (addr < heap_start + sizeof(kheap_header_t) || addr >= heap_end) {
        return;
    }

    kheap_header_t *header = (kheap_header_t *)((uint8_t *)ptr - sizeof(kheap_header_t));

    /* Kiểm tra tính toàn vẹn (magic number) */
    if (header->magic != KHEAP_MAGIC) {
        return;
    }

    uint32_t start_block = header->start_block;
    uint32_t num_blocks = header->num_blocks;

    /* Xóa các bit tương ứng trong bitmap để trả lại trạng thái Free */
    for (uint32_t b = start_block; b < start_block + num_blocks; b++) {
        if (b < KHEAP_TOTAL_BLOCKS) {
            bitmap_clear(b);
        }
    }

    /* Hủy magic để phát hiện lỗi giải phóng lặp (double free) */
    header->magic = 0;
}

/**
 * kheap_get_total_memory:
 * Trả về tổng dung lượng heap (bytes).
 */
size_t kheap_get_total_memory(void)
{
    return KHEAP_SIZE;
}

/**
 * kheap_get_used_memory:
 * Trả về tổng số byte đang được cấp phát (tính theo block).
 */
size_t kheap_get_used_memory(void)
{
    size_t used_blocks = 0;
    for (uint32_t i = 0; i < KHEAP_TOTAL_BLOCKS; i++) {
        if (bitmap_test(i)) {
            used_blocks++;
        }
    }
    return used_blocks * KHEAP_BLOCK_SIZE;
}

/**
 * kheap_get_free_memory:
 * Trả về tổng số byte còn trống trong heap.
 */
size_t kheap_get_free_memory(void)
{
    return kheap_get_total_memory() - kheap_get_used_memory();
}

/**
 * kheap_print_stats:
 * In thông số chi tiết trạng thái heap ra màn hình Framebuffer và Serial COM1.
 */
void kheap_print_stats(void)
{
    size_t total = kheap_get_total_memory();
    size_t used  = kheap_get_used_memory();
    size_t free  = kheap_get_free_memory();

    /* In thông tin thống kê tóm tắt */
    const char *msg1 = "[ KHEAP ] Total: ";
    fb_write(msg1, 17);
    serial_write(SERIAL_COM1_BASE, msg1, 17);

    /* In ra số KB (total / 1024) */
    char buf[16];
    uint32_t total_kb = total / 1024;
    uint32_t used_kb = used / 1024;
    uint32_t free_kb = free / 1024;

    /* In chuỗi đơn giản */
    buf[0] = '0' + (char)(total_kb / 100);
    buf[1] = '0' + (char)((total_kb / 10) % 10);
    buf[2] = '0' + (char)(total_kb % 10);
    buf[3] = ' '; buf[4] = 'K'; buf[5] = 'B'; buf[6] = ','; buf[7] = ' ';
    buf[8] = 'U'; buf[9] = 's'; buf[10] = 'e'; buf[11] = 'd'; buf[12] = ':'; buf[13] = ' ';
    fb_write(buf, 14);
    serial_write(SERIAL_COM1_BASE, buf, 14);

    buf[0] = '0' + (char)(used_kb / 100);
    buf[1] = '0' + (char)((used_kb / 10) % 10);
    buf[2] = '0' + (char)(used_kb % 10);
    buf[3] = ' '; buf[4] = 'K'; buf[5] = 'B'; buf[6] = ','; buf[7] = ' ';
    buf[8] = 'F'; buf[9] = 'r'; buf[10] = 'e'; buf[11] = 'e'; buf[12] = ':'; buf[13] = ' ';
    fb_write(buf, 14);
    serial_write(SERIAL_COM1_BASE, buf, 14);

    buf[0] = '0' + (char)(free_kb / 100);
    buf[1] = '0' + (char)((free_kb / 10) % 10);
    buf[2] = '0' + (char)(free_kb % 10);
    buf[3] = ' '; buf[4] = 'K'; buf[5] = 'B'; buf[6] = '\n';
    fb_write(buf, 7);
    serial_write(SERIAL_COM1_BASE, buf, 7);
}
