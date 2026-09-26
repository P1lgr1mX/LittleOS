#ifndef KERNEL_KHEAP_H
#define KERNEL_KHEAP_H

#include "types.h"

/* Granular allocation block size in the bitmap heap (32 bytes) */
#define KHEAP_BLOCK_SIZE     32

/* Total dedicated kernel heap pool capacity (256 KB) */
#define KHEAP_SIZE           (256 * 1024)

/* Total number of tracking blocks available in the heap */
#define KHEAP_TOTAL_BLOCKS   (KHEAP_SIZE / KHEAP_BLOCK_SIZE)

/* Size of the allocation tracking bitmap in bytes (1 bit per block) */
#define KHEAP_BITMAP_SIZE    (KHEAP_TOTAL_BLOCKS / 8)

/* Magic sentinel identifying valid allocated block headers */
#define KHEAP_MAGIC          0x1EA9BEEF

/*
 * Metadata header prepended to every memory chunk allocated via kmalloc
 */
typedef struct kheap_header {
    uint32_t magic;         /* KHEAP_MAGIC integrity verification signature */
    uint32_t start_block;   /* Starting block index in tracking bitmap */
    uint32_t num_blocks;    /* Number of contiguous allocated blocks */
    size_t   size;          /* Requested payload size in bytes */
} __attribute__((packed)) kheap_header_t;

/* Initialize kernel bitmap heap pool and tracking structures */
void kheap_init(void);

/* Allocate contiguous memory from the kernel heap */
void *kmalloc(size_t size);

/* Allocate zero-initialized memory block */
void *kcalloc(size_t num, size_t size);

/* Resize an existing dynamically allocated memory block */
void *krealloc(void *ptr, size_t new_size);

/* Release an allocated memory block back to the heap pool */
void kfree(void *ptr);

/* Memory profiling and heap diagnostics */
size_t kheap_get_total_memory(void);
size_t kheap_get_used_memory(void);
size_t kheap_get_free_memory(void);
void kheap_print_stats(void);

#endif /* KERNEL_KHEAP_H */
