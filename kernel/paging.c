#include "paging.h"

/* Các cấu trúc phân trang được định nghĩa và căn chỉnh 4KB trong boot/loader.s */
extern unsigned int boot_page_directory[1024];
extern unsigned int boot_page_table1[1024];

/**
 * paging_init:
 * Kiểm tra và đảm bảo cấu hình phân trang cho Higher-Half Kernel:
 * - Entry 0 (0MB - 4MB ảo): Ánh xạ Identity Map tới 0MB - 4MB vật lý.
 * - Entry 768 (3GB - 3GB+4MB ảo): Ánh xạ Higher-Half tới 0MB - 4MB vật lý.
 */
void paging_init(void)
{
    /* Lấy địa chỉ vật lý của boot_page_table1 bằng cách trừ đi 0xC0000000 */
    unsigned int pt_phys = VIRTUAL_TO_PHYSICAL(boot_page_table1);

    /*
     * Bit 0: Present = 1 (trang có mặt trong RAM)
     * Bit 1: Read/Write = 1 (cho phép đọc và ghi)
     * Bit 2: Supervisor = 0 (chỉ Kernel mode Ring 0 mới được truy cập)
     */
    boot_page_directory[0]   = pt_phys | 3;
    boot_page_directory[768] = pt_phys | 3;
}