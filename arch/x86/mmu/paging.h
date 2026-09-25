#ifndef ARCH_X86_PAGING_H
#define ARCH_X86_PAGING_H

#include "types.h"

/* Địa chỉ ảo cơ sở của Higher-Half Kernel (mốc 3GB) */
#define KERNEL_VIRTUAL_BASE 0xC0000000

/* Macro chuyển đổi giữa địa chỉ ảo và địa chỉ vật lý trong vùng kernel */
#define VIRTUAL_TO_PHYSICAL(addr) ((uint32_t)(addr) - KERNEL_VIRTUAL_BASE)
#define PHYSICAL_TO_VIRTUAL(addr) ((uint32_t)(addr) + KERNEL_VIRTUAL_BASE)

/* Cờ thuộc tính trang x86 */
#define PAGING_FLAG_PRESENT  0x01
#define PAGING_FLAG_WRITE    0x02
#define PAGING_FLAG_USER     0x04

#define PAGING_KERNEL_PAGE   (PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE)                    /* 0x03 */
#define PAGING_USER_PAGE     (PAGING_FLAG_PRESENT | PAGING_FLAG_WRITE | PAGING_FLAG_USER) /* 0x07 */

/* Điểm vào và đỉnh ngăn xếp mặc định cho tiến trình User theo Chapter 11 */
#define USER_STACK_TOP       0xBFFFFFFB
#define USER_ENTRY_POINT     0x00000000

/* Khởi tạo và đồng bộ trạng thái phân trang Higher-Half */
void paging_init(void);

/* Thiết lập thư mục và bảng trang cho tiến trình User, nạp mã chương trình */
void paging_setup_user_process(uint32_t module_start, uint32_t module_size);

/* Hàm Assembly nhảy sang User Mode trong arch/x86/boot/loader.s */
void enter_user_mode(uint32_t eip, uint32_t user_esp);

#endif /* ARCH_X86_PAGING_H */
