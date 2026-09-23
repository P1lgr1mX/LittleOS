#ifndef SYSCALL_H
#define SYSCALL_H

#define ENOSYS 38    

#define SYS_EXIT  1 
#define SYS_FORK  2
#define SYS_READ  3
#define SYS_WRITE 4

#ifndef _UINT32_T_DECLARED
#define _UINT32_T_DECLARED
typedef unsigned int uint32_t;
#endif

/*
 * Cấu trúc con trỏ ngăn xếp pt_regs khớp chính xác 100% với
 * thứ tự push trong Int128Handler (syscall.s)
 */
struct pt_regs {
    /* Đẩy thủ công bởi Assembly (đỉnh stack hiện tại) */
    uint32_t ebx;       /* offset 0  */
    uint32_t ecx;       /* offset 4  */
    uint32_t edx;       /* offset 8  */
    uint32_t esi;       /* offset 12 */
    uint32_t edi;       /* offset 16 */
    uint32_t ebp;       /* offset 20 */
    uint32_t orig_eax;  /* offset 24 (lưu -ENOSYS) */
    uint32_t ds;        /* offset 28 */
    uint32_t es;        /* offset 32 */
    uint32_t fs;        /* offset 36 */
    uint32_t gs;        /* offset 40 */
    uint32_t eax;       /* offset 44 (số hiệu syscall từ User / giá trị trả về) */

    /* CPU tự động đẩy khi chuyển từ Ring 3 sang Ring 0 */
    uint32_t eip;       /* offset 48 */
    uint32_t cs;        /* offset 52 */
    uint32_t eflags;    /* offset 56 */
    uint32_t esp;       /* offset 60 */
    uint32_t ss;        /* offset 64 */
} __attribute__((packed));

void do_syscall_in_C(struct pt_regs *regs); 
void Int128Handler(void);
void syscall_init(void);

#endif /* SYSCALL_H */