#include "kernel/syscall.h"
#include "drivers/framebuffer.h"
#include "drivers/serial.h"
#include "drivers/keyboard.h"
#include "arch/x86/io.h"
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
            regs->eax = count;
        } else {
            regs->eax = (uint32_t)-1;
        }
        break;
    }

    case SYS_EXIT:
        /* ebx: mã thoát (exit code) */
        regs->eax = regs->ebx;
        break;

    case SYS_CLEAR:
        fb_clear();
        serial_write(SERIAL_COM1_BASE, "\033[2J\033[H", 7);
        regs->eax = 0;
        break;

    case SYS_READ: {
        /*
         * ebx: file descriptor (0 = stdin)
         * ecx: con trỏ buffer của User Mode (char *buf)
         * edx: dung lượng tối đa của buffer (uint32_t max_count)
         *
         * Chế độ Canonical (Line / Buffer mode):
         * Gom toàn bộ ký tự vào buffer cho đến khi nhận Enter ('\n', '\r')
         * hoặc ký tự kết thúc '\0'. Tự động thêm '\0' vào cuối buffer
         * và trả về toàn bộ buffer hoàn chỉnh cho User Mode (Ring 3).
         */
        if (regs->ebx == 0) {
            char *buf = (char *)regs->ecx;
            uint32_t max_count = regs->edx;
            if (!buf || max_count <= 1) {
                if (buf && max_count == 1) {
                    buf[0] = '\0';
                }
                regs->eax = 0;
                break;
            }

            enable_interrupts();
            uint32_t idx = 0;

            /* Lặp gom ký tự cho đến khi đầy buffer hoặc gặp ký tự kết thúc (\n, \r, \0) */
            while (idx < max_count - 1) {
                int c = keyboard_getchar();
                if (c == -1) {
                    c = serial_read_char(SERIAL_COM1_BASE);
                }

                if (c == -1) {
                    /* Chưa có phím mới: CPU ngủ để đợi ngắt phần cứng tiếp theo */
                    __asm__ volatile("hlt");
                    continue;
                }

                /* Khi gặp Enter (\n, \r) hoặc \0: Hoàn tất buffer */
                if (c == '\0' || c == '\n' || c == '\r') {
                    /* Xuống dòng hiển thị */
                    fb_write("\n", 1);
                    serial_write(SERIAL_COM1_BASE, "\r\n", 2);
                    break;
                }

                /* Xử lý phím Backspace ('\b' hoặc 127 DEL) */
                if (c == '\b' || c == 127) {
                    if (idx > 0) {
                        idx--;
                        /* Xóa 1 ký tự trên Framebuffer */
                        fb_write("\b", 1);
                        /* Xóa 1 ký tự trên Serial terminal */
                        serial_write(SERIAL_COM1_BASE, "\b \b", 3);
                    }
                    continue;
                }

                /* Lưu ký tự vào buffer của User Mode */
                buf[idx++] = (char)c;

                /* Echo ký tự người dùng vừa gõ ra màn hình và serial */
                char echo[2];
                echo[0] = (char)c;
                echo[1] = '\0';
                fb_write(echo, 1);
                serial_write_char(SERIAL_COM1_BASE, (char)c);
            }

            /* Đảm bảo buffer luôn kết thúc bằng '\0' chuẩn chuỗi C */
            buf[idx] = '\0';

            /* Trả về độ dài chuỗi ký tự hợp lệ đã nhận */
            regs->eax = idx;
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
