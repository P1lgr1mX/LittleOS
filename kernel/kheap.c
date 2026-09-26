/*
 * Kernel Bitmap Heap Allocator
 * Part of the AetherOS Operating System Project.
 * Licensed under the BSD 3-Clause License.
 */
#include "kernel/kheap.h"
#include "drivers/framebuffer.h"
#include "drivers/serial.h"

/* Statically allocated heap memory pool in .bss, aligned to a 4KB boundary */
static uint8_t kheap_memory[KHEAP_SIZE] __attribute__((aligned(4096)));

/* Bitmap tracking the allocation status of each block (0 = Free, 1 = Allocated) */
static uint8_t kheap_bitmap[KHEAP_BITMAP_SIZE];

/* Initialization status flag */
static int kheap_initialized = 0;

/* Freestanding low-level memory manipulation primitives */
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

/* Bitmap bitwise manipulation helpers */
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
 * Initializes the kernel heap pool and resets the tracking bitmap.
 * All blocks are marked as free (bit = 0).
 */
void kheap_init(void)
{
    kheap_memset(kheap_bitmap, 0, sizeof(kheap_bitmap));
    kheap_memset(kheap_memory, 0, sizeof(kheap_memory));
    kheap_initialized = 1;
}

/**
 * kmalloc:
 * Allocates contiguous heap memory using a first-fit search algorithm on the bitmap.
 *
 * @param size Number of requested payload bytes
 * @return Pointer to the allocated payload memory, or NULL if allocation fails
 */
void *kmalloc(size_t size)
{
    if (!kheap_initialized || size == 0) {
        return NULL;
    }

    /* Compute total required size including the management header */
    size_t total_needed = sizeof(kheap_header_t) + size;
    uint32_t blocks_needed = (uint32_t)((total_needed + KHEAP_BLOCK_SIZE - 1) / KHEAP_BLOCK_SIZE);

    if (blocks_needed > KHEAP_TOTAL_BLOCKS) {
        return NULL;
    }

    /* Scan bitmap for a contiguous sequence of free blocks */
    uint32_t consecutive_free = 0;
    uint32_t start_block = 0;

    for (uint32_t i = 0; i < KHEAP_TOTAL_BLOCKS; i++) {
        if (!bitmap_test(i)) {
            if (consecutive_free == 0) {
                start_block = i;
            }
            consecutive_free++;

            if (consecutive_free == blocks_needed) {
                /* Mark selected blocks as allocated in bitmap */
                for (uint32_t b = start_block; b < start_block + blocks_needed; b++) {
                    bitmap_set(b);
                }

                /* Populate allocation metadata header */
                kheap_header_t *header = (kheap_header_t *)&kheap_memory[start_block * KHEAP_BLOCK_SIZE];
                header->magic = KHEAP_MAGIC;
                header->start_block = start_block;
                header->num_blocks = blocks_needed;
                header->size = size;

                /* Return pointer immediately following the header */
                return (void *)((uint8_t *)header + sizeof(kheap_header_t));
            }
        } else {
            consecutive_free = 0;
        }
    }

    return NULL; /* Out of memory or contiguous blocks unavailable */
}

/**
 * kcalloc:
 * Allocates memory for an array of num elements of size bytes, zero-initializing the space.
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
 * Resizes an allocated memory block, preserving existing data contents.
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

    /* Check if current allocated block capacity already satisfies new_size */
    size_t current_capacity = (header->num_blocks * KHEAP_BLOCK_SIZE) - sizeof(kheap_header_t);
    if (new_size <= current_capacity) {
        header->size = new_size;
        return ptr;
    }

    /* Allocate a new memory block and transfer old contents */
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
 * Deallocates a memory block previously allocated by kmalloc/kcalloc/krealloc.
 */
void kfree(void *ptr)
{
    if (!ptr || !kheap_initialized) {
        return;
    }

    uintptr_t addr = (uintptr_t)ptr;
    uintptr_t heap_start = (uintptr_t)kheap_memory;
    uintptr_t heap_end = heap_start + KHEAP_SIZE;

    /* Verify that pointer falls within valid kernel heap pool bounds */
    if (addr < heap_start + sizeof(kheap_header_t) || addr >= heap_end) {
        return;
    }

    kheap_header_t *header = (kheap_header_t *)((uint8_t *)ptr - sizeof(kheap_header_t));

    /* Verify integrity signature to detect corruption or invalid pointers */
    if (header->magic != KHEAP_MAGIC) {
        return;
    }

    uint32_t start_block = header->start_block;
    uint32_t num_blocks = header->num_blocks;

    /* Reset corresponding bits in bitmap to mark blocks as free */
    for (uint32_t b = start_block; b < start_block + num_blocks; b++) {
        if (b < KHEAP_TOTAL_BLOCKS) {
            bitmap_clear(b);
        }
    }

    /* Invalidate magic number to guard against double-free errors */
    header->magic = 0;
}

/**
 * kheap_get_total_memory:
 * Returns the total pool size of the kernel heap in bytes.
 */
size_t kheap_get_total_memory(void)
{
    return KHEAP_SIZE;
}

/**
 * kheap_get_used_memory:
 * Returns the total allocated memory in bytes based on occupied blocks.
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
 * Returns the total unallocated memory in bytes available in the heap pool.
 */
size_t kheap_get_free_memory(void)
{
    return kheap_get_total_memory() - kheap_get_used_memory();
}

/**
 * kheap_print_stats:
 * Outputs memory statistics to the console framebuffer and COM1 serial port.
 */
void kheap_print_stats(void)
{
    size_t total = kheap_get_total_memory();
    size_t used  = kheap_get_used_memory();
    size_t free  = kheap_get_free_memory();

    const char *header_msg = "[ KHEAP ] Total: ";
    fb_write(header_msg, 17);
    serial_write(SERIAL_COM1_BASE, header_msg, 17);

    char buf[16];
    uint32_t total_kb = total / 1024;
    uint32_t used_kb = used / 1024;
    uint32_t free_kb = free / 1024;

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
