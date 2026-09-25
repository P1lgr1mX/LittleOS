#include "arch/x86/mmu/paging.h"

/* Các cấu trúc phân trang được định nghĩa và căn chỉnh 4KB trong arch/x86/boot/loader.s */
extern uint32_t boot_page_directory[1024];
extern uint32_t boot_page_table1[1024];

/* Hàm nạp CR3 trong arch/x86/boot/loader.s */
extern void load_page_directory(uint32_t cr3);

/* Dung lượng tối đa cấp cho chương trình người dùng (16 trang * 4KB = 64KB) */
#define USER_CODE_MAX_PAGES 16

/* Khung trang (Page frames) và bảng trang dành cho User Process căn chỉnh 4KB */
static uint32_t  user_page_directory[1024]                  __attribute__((aligned(4096)));
static uint32_t  user_code_page_table[1024]                 __attribute__((aligned(4096)));
static uint32_t  user_stack_page_table[1024]                __attribute__((aligned(4096)));
static uint8_t   user_code_pages[USER_CODE_MAX_PAGES][4096] __attribute__((aligned(4096)));
static uint8_t   user_stack_page[4096]                      __attribute__((aligned(4096)));

/**
 * paging_init:
 * Kiểm tra và đảm bảo cấu hình phân trang cho Higher-Half Kernel:
 * - Entry 0 (0MB - 4MB ảo): Ánh xạ Identity Map tới 0MB - 4MB vật lý.
 * - Entry 768 (3GB - 3GB+4MB ảo): Ánh xạ Higher-Half tới 0MB - 4MB vật lý.
 */
void paging_init(void)
{
    /* Lấy địa chỉ vật lý của boot_page_table1 bằng cách trừ đi 0xC0000000 */
    uint32_t pt_phys = VIRTUAL_TO_PHYSICAL(boot_page_table1);

    /*
     * Bit 0: Present = 1 (trang có mặt trong RAM)
     * Bit 1: Read/Write = 1 (cho phép đọc và ghi)
     * Bit 2: Supervisor = 0 (chỉ Kernel mode Ring 0 mới được truy cập)
     */
    boot_page_directory[0]   = pt_phys | PAGING_KERNEL_PAGE;
    boot_page_directory[768] = pt_phys | PAGING_KERNEL_PAGE;
}

/**
 * paging_setup_user_process:
 * Cấu hình không gian địa chỉ ảo và nạp mã chương trình người dùng (Chapter 11):
 * 1. Toàn bộ 64KB (16 trang) được ánh xạ sẵn tại 0x00000000 (.text, .data, .bss).
 * 2. Ngăn xếp User (Stack) đặt tại 0xBFFFFFFB (Entry 767 trong Page Directory).
 * 3. Bảo toàn vùng nhớ Kernel tại 0xC0000000 (Entry 768) với quyền Supervisor.
 * 4. Nạp địa chỉ vật lý của user_page_directory vào CR3.
 */
void paging_setup_user_process(uint32_t module_start, uint32_t module_size)
{
    /* Xóa sạch các bảng trang và frame */
    for (int i = 0; i < 1024; i++) {
        user_page_directory[i]   = 0;
        user_code_page_table[i]  = 0;
        user_stack_page_table[i] = 0;
    }
    for (int p = 0; p < USER_CODE_MAX_PAGES; p++) {
        for (int i = 0; i < 4096; i++) {
            user_code_pages[p][i] = 0;
        }
    }
    for (int i = 0; i < 4096; i++) {
        user_stack_page[i] = 0;
    }

    /* Sao chép mã thực thi từ module vào page frames */
    uint8_t *src = (uint8_t *)module_start;
    for (uint32_t p = 0; p < USER_CODE_MAX_PAGES; p++) {
        uint32_t offset = p * 4096;
        if (offset < module_size) {
            uint32_t to_copy = 4096;
            if (offset + to_copy > module_size) {
                to_copy = module_size - offset;
            }
            for (uint32_t i = 0; i < to_copy; i++) {
                user_code_pages[p][i] = src[offset + i];
            }
        }

        /* Ánh xạ đầy đủ 64KB User Code/Data/BSS bắt đầu từ địa chỉ ảo 0x00000000 */
        user_code_page_table[p] = VIRTUAL_TO_PHYSICAL(user_code_pages[p]) | PAGING_USER_PAGE;
    }
    user_page_directory[0] = VIRTUAL_TO_PHYSICAL(user_code_page_table) | PAGING_USER_PAGE;

    /* 2. Ánh xạ User Stack tại địa chỉ ảo 0xBFFFFFFB */
    user_stack_page_table[1023] = VIRTUAL_TO_PHYSICAL(user_stack_page) | PAGING_USER_PAGE;
    user_page_directory[767]    = VIRTUAL_TO_PHYSICAL(user_stack_page_table) | PAGING_USER_PAGE;

    /* 3. Giữ nguyên vùng nhớ Kernel ở Higher-Half (0xC0000000 trở lên) */
    user_page_directory[768] = boot_page_directory[768];

    /* 4. Kích hoạt bảng phân trang của User bằng cách nạp địa chỉ vật lý vào CR3 */
    load_page_directory(VIRTUAL_TO_PHYSICAL(user_page_directory));
}
