#include "arch/x86/isr.h"
#include "drivers/pic.h"
#include "drivers/framebuffer.h"

static isr_handler_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_handler_t handler)
{
    interrupt_handlers[n] = handler;
}

void interrupt_handler(struct registers *cpu, struct stack_state *stack, uint32_t interrupt)
{
    (void)cpu;
    (void)stack;

    if (interrupt < 256 && interrupt_handlers[interrupt] != 0) {
        interrupt_handlers[interrupt](cpu, stack, interrupt);
    } else {
        /* Handle unhandled CPU exceptions (vectors 0 - 31) */
        if (interrupt < 32) {
            fb_set_color(FB_LIGHT_RED, FB_BLACK);
            write("\n[EXCEPTION] Unhandled CPU Exception: ", 37);
            char buf[4];
            if (interrupt >= 10) {
                buf[0] = '0' + (char)(interrupt / 10);
                buf[1] = '0' + (char)(interrupt % 10);
                buf[2] = '\n';
                buf[3] = '\0';
                write(buf, 3);
            } else {
                buf[0] = '0' + (char)interrupt;
                buf[1] = '\n';
                buf[2] = '\0';
                write(buf, 2);
            }
        }
    }

    /* Send End-Of-Interrupt (EOI) signal to 8259 PIC for hardware IRQs (vectors 32 - 47) */
    if (interrupt >= 32 && interrupt <= 47) {
        pic_ack((uint8_t)interrupt);
    }
}
