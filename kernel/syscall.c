#include "kernel/syscall.h"
#include "drivers/framebuffer.h"
#include "drivers/serial.h"
#include "arch/x86/idt.h"

void do_syscall_in_C(struct pt_regs *regs)
{
    if (!regs) {
        return;
    }

    switch (regs->eax) {
    case SYS_WRITE: {
        /*
         * ebx: file descriptor (1 = stdout, 2 = stderr)
         * ecx: con trỏ chuỗi (const char *buf)
         * edx: số byte cần ghi (uint32_t count)
         */
        if (regs->ebx == 1 || regs->ebx == 2) {
            const char *buf = (const char *)regs->ecx;
            uint32_t count = regs->edx;
            if (buf && count > 0) { 
                fb_write(buf, count);
                serial_write(SERIAL_COM1_BASE, buf, count);
            }
            regs->eax = count; /* Ghi ra số byte đã ghi thành công */
        } else {
            regs->eax = (uint32_t)-1;
        }
        break;
    }

    case SYS_EXIT:
        /* ebx: mã thoát (exit code) */
        regs->eax = regs->ebx;
        break;

    case SYS_READ: {
        /*
         * ebx: file descriptor (0 = stdin)
         * ecx: con trỏ buffer của User Mode (char *buf)
         * edx: kích thước bộ đệm (uint32_t count)
         */
        if (regs->ebx == 0) {
            char *buf = (char *)regs->ecx;
            uint32_t count = regs->edx;
            if (!buf || count == 0) {
                regs->eax = 0;
                break;
            }
            int c = serial_read_char(SERIAL_COM1_BASE);
            if (c != -1) {
                buf[0] = (char)c;
                regs->eax = 1;
            } else {
                regs->eax = 0;
            }
        } else {
            regs->eax = (uint32_t)-1;
        }
        break;
    }

    case SYS_FORK:
    default:
        regs->eax = (uint32_t)-ENOSYS;
        break;
    }
}

void syscall_init(void)
{
    /* Cổng 0x80 (128): 32-bit Interrupt Gate, DPL = 3 (0xEE) để User Mode (Ring 3) có quyền gọi */
    idt_set_gate(0x80, (uint32_t)Int128Handler, 0x08, 0xEE);
}
