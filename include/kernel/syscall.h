#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include "types.h"

#define ENOSYS 38    

#define SYS_EXIT  1 
#define SYS_FORK  2
#define SYS_READ  3
#define SYS_WRITE 4
#define SYS_CLEAR 5

/*
 * Register state structure pushed on the kernel stack, matching
 * exactly the layout constructed by Int128Handler in kernel/syscall.s.
 */
struct pt_regs {
    /* Pushed by software stub in kernel/syscall.s */
    uint32_t ebx;       /* offset 0: syscall argument 1 */
    uint32_t ecx;       /* offset 4: syscall argument 2 */
    uint32_t edx;       /* offset 8: syscall argument 3 */
    uint32_t esi;       /* offset 12 */
    uint32_t edi;       /* offset 16 */
    uint32_t ebp;       /* offset 20 */
    uint32_t orig_eax;  /* offset 24: default -ENOSYS error code */
    uint32_t ds;        /* offset 28: user data segment selector */
    uint32_t es;        /* offset 32 */
    uint32_t fs;        /* offset 36 */
    uint32_t gs;        /* offset 40 */
    uint32_t eax;       /* offset 44: syscall number / return value */

    /* Pushed automatically by IA-32 CPU hardware upon Ring 3 -> Ring 0 transition */
    uint32_t eip;       /* offset 48: user program counter */
    uint32_t cs;        /* offset 52: user code segment selector */
    uint32_t eflags;    /* offset 56: user CPU flags */
    uint32_t esp;       /* offset 60: user stack pointer */
    uint32_t ss;        /* offset 64: user stack segment selector */
} __attribute__((packed));

void do_syscall_in_C(struct pt_regs *regs); 
void Int128Handler(void);
void syscall_init(void);

#endif /* KERNEL_SYSCALL_H */
