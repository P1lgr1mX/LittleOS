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
         * ecx: buffer pointer (const char *buf)
         * edx: byte count (uint32_t count)
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
        /* ebx: exit status code */
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
         * ecx: userland buffer pointer (char *buf)
         * edx: maximum buffer capacity (uint32_t max_count)
         *
         * Canonical line-buffered input mode:
         * Gathers characters until an Enter key ('\n', '\r') or null
         * terminator is encountered. Null-terminates the buffer and
         * returns the length of accepted characters to Ring 3 userland.
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

            /* Buffer characters until maximum count is reached or delimiter received */
            while (idx < max_count - 1) {
                int c = keyboard_getchar();
                if (c == -1) {
                    c = serial_read_char(SERIAL_COM1_BASE);
                }

                if (c == -1) {
                    /* Wait for next hardware interrupt event */
                    __asm__ volatile("hlt");
                    continue;
                }

                /* Enter ('\n', '\r') or null byte signals end-of-line */
                if (c == '\0' || c == '\n' || c == '\r') {
                    fb_write("\n", 1);
                    serial_write(SERIAL_COM1_BASE, "\r\n", 2);
                    break;
                }

                /* Handle backspace ('\b' or DEL 127) */
                if (c == '\b' || c == 127) {
                    if (idx > 0) {
                        idx--;
                        fb_write("\b", 1);
                        serial_write(SERIAL_COM1_BASE, "\b \b", 3);
                    }
                    continue;
                }

                /* Store character into userland buffer */
                buf[idx++] = (char)c;

                /* Echo input character to console and serial terminal */
                char echo[2];
                echo[0] = (char)c;
                echo[1] = '\0';
                fb_write(echo, 1);
                serial_write_char(SERIAL_COM1_BASE, (char)c);
            }

            /* Guarantee null termination */
            buf[idx] = '\0';

            /* Return number of characters in the received line */
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
    /* Interrupt vector 0x80 (128): 32-bit Interrupt Gate, DPL = 3 (0xEE) for userland access */
    idt_set_gate(0x80, (uint32_t)Int128Handler, 0x08, 0xEE);
}
