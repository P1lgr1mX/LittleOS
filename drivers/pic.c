#include "pic.h"
#include "io.h"

void pic_ack(unsigned char irq)
{
    if (irq < PIC1_START_INTERRUPT || irq > PIC2_END_INTERRUPT) {
        return; 
    }
    if (irq >= PIC2_START_INTERRUPT) {
        outb(PIC2_PORT_A, PIC_ACK);
    }
    outb(PIC1_PORT_A, PIC_ACK);
}

void pic_set_mask(unsigned char mask1, unsigned char mask2)
{
    outb(PIC1_PORT_B, mask1);
    outb(PIC2_PORT_B, mask2);
}

void pic_remap(void)
{
    /* ICW1: Bắt đầu chuỗi khởi tạo PIC1 và PIC2 */
    outb(PIC1_PORT_A, 0x11);
    outb(PIC2_PORT_A, 0x11);

    /* ICW2: Đặt vector ngắt bắt đầu: PIC1 = 0x20 (32), PIC2 = 0x28 (40) */
    outb(PIC1_PORT_B, PIC1_START_INTERRUPT);
    outb(PIC2_PORT_B, PIC2_START_INTERRUPT);

    /* ICW3: Thiết lập kết nối cascade giữa PIC1 và PIC2 */
    outb(PIC1_PORT_B, 0x04); /* PIC1 có slave tại IRQ 2 */
    outb(PIC2_PORT_B, 0x02); /* PIC2 kết nối tới IRQ 2 của PIC1 */

    /* ICW4: Chế độ hoạt động 8086/88 */
    outb(PIC1_PORT_B, 0x01);
    outb(PIC2_PORT_B, 0x01);

    /* Mặt nạ ngắt ban đầu:
     * Cho phép IRQ 1 (Keyboard: bit 1 = 0)
     * PIC1: 0xFD (1111 1101b - chỉ mở IRQ1)
     * PIC2: 0xFF (tất cả khóa)
     */
    pic_set_mask(0xFD, 0xFF);
}