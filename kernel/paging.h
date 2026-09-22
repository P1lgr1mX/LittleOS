#ifndef PAGING_H
#define PAGING_H

/* Địa chỉ ảo cơ sở của Higher-Half Kernel (mốc 3GB) */
#define KERNEL_VIRTUAL_BASE 0xC0000000

/* Macro chuyển đổi giữa địa chỉ ảo và địa chỉ vật lý trong vùng kernel */
#define VIRTUAL_TO_PHYSICAL(addr) ((unsigned int)(addr) - KERNEL_VIRTUAL_BASE)
#define PHYSICAL_TO_VIRTUAL(addr) ((unsigned int)(addr) + KERNEL_VIRTUAL_BASE)

/* Khởi tạo và đồng bộ trạng thái phân trang Higher-Half */
void paging_init(void);

#endif /* PAGING_H */
