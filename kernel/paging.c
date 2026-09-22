#include "paging.h"

/* Các hàm hợp ngữ từ boot/loader.s */
extern void load_page_directory(unsigned int *page_directory);
extern void enable_paging(void);

/* Thư mục trang (Page Directory) và bảng trang (Page Table) căn chỉnh 4KB */
static unsigned int page_directory[1024] __attribute__((aligned(4096)));
static unsigned int first_page_table[1024] __attribute__((aligned(4096)));

void paging_init(void)
{
    /* 1. Khởi tạo toàn bộ PDE: Read/Write, Supervisor, Not Present (0x00000002) */
    for (int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }

    /* 2. Thiết lập bảng trang đầu tiên: Identity mapping 4MB đầu tiên (0x0 -> 0x3FFFFF)
          Mỗi trang có kích thước 4KB (0x1000)
          Cờ 3 (0b011): Present = 1, Read/Write = 1, Supervisor (PL0) = 0 */
    for (unsigned int i = 0; i < 1024; i++) {
        first_page_table[i] = (i * 0x1000) | 3;
    }

    /* 3. Gán bảng trang đầu tiên vào PDE 0 (quản lý không gian ảo 0MB - 4MB) */
    page_directory[0] = ((unsigned int)first_page_table) | 3;

    /* 4. Nạp địa chỉ của page_directory vào thanh ghi CR3 */
    load_page_directory(page_directory);

    /* 5. Kích hoạt phân trang bằng cách bật bit PG (bit 31) trong thanh ghi CR0 */
    enable_paging();
}