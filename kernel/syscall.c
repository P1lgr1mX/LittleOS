#include "syscall.h"
#include "framebuffer.h"
#include "serial.h"
#include "idt.h"

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
         * edx: số byte cần ghi (unsigned int count)
         */
        if (regs->ebx == 1 || regs->ebx == 2) {
            const char *buf = (const char *)regs->ecx; //truy cap con tro chưa được đẩy 
            uint32_t count = regs->edx; //số byte cần ghi
            if (buf && count > 0) { 
                fb_write(buf, count);
                serial_write(SERIAL_COM1_BASE, buf, count);
            }
            regs->eax = count; //ghi ra số byte ghi
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
                regs->eax = 1; /* Đã đọc thành công 1 byte */
            } else {
                regs->eax = 0; /* Chưa có ký tự nào sẵn sàng */
            }
        } else {
            regs->eax = (uint32_t)-1; /* Sai file descriptor */
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
    idt_set_gate(0x80, (unsigned int)Int128Handler, 0x08, 0xEE);
}
