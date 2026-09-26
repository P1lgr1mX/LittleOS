#include "drivers/pic.h"
#include "arch/x86/io.h"

void pic_ack(uint8_t irq)
{
    if (irq < PIC1_START_INTERRUPT || irq > PIC2_END_INTERRUPT) {
        return; 
    }
    if (irq >= PIC2_START_INTERRUPT) {
        outb(PIC2_PORT_A, PIC_ACK);
    }
    outb(PIC1_PORT_A, PIC_ACK);
}

void pic_set_mask(uint8_t mask1, uint8_t mask2)
{
    outb(PIC1_PORT_B, mask1);
    outb(PIC2_PORT_B, mask2);
}

void pic_remap(void)
{
    /* ICW1: Initialize Master and Slave 8259 PICs in cascade mode */
    outb(PIC1_PORT_A, 0x11);
    outb(PIC2_PORT_A, 0x11);

    /* ICW2: Set base vector offsets: Master = 0x20 (32), Slave = 0x28 (40) */
    outb(PIC1_PORT_B, PIC1_START_INTERRUPT);
    outb(PIC2_PORT_B, PIC2_START_INTERRUPT);

    /* ICW3: Establish cascading configuration */
    outb(PIC1_PORT_B, 0x04); /* Master PIC: Slave attached to IRQ 2 */
    outb(PIC2_PORT_B, 0x02); /* Slave PIC: Cascade identity on IRQ 2 */

    /* ICW4: Select 8086/88 operation mode */
    outb(PIC1_PORT_B, 0x01);
    outb(PIC2_PORT_B, 0x01);

    /* Initial Interrupt Mask Register (IMR) configuration:
     * Unmask IRQ 1 (PS/2 Keyboard: bit 1 = 0) and IRQ 4 (COM1 Serial: bit 4 = 0)
     * Master PIC: ~(0x02 | 0x10) = 0xED (1110 1101b)
     * Slave PIC: 0xFF (all IRQ lines masked)
     */
    pic_set_mask(0xED, 0xFF);
}