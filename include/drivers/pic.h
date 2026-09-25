#ifndef DRIVERS_PIC_H
#define DRIVERS_PIC_H

#include "types.h"

#define PIC1_PORT_A 0x20 /* PIC1 Command / Status port */
#define PIC1_PORT_B 0x21 /* PIC1 Data / Mask port */
#define PIC2_PORT_A 0xA0 /* PIC2 Command / Status port */
#define PIC2_PORT_B 0xA1 /* PIC2 Data / Mask port */

#define PIC1_START_INTERRUPT 0x20 /* 8 ngắt đầu (32..39) */
#define PIC2_START_INTERRUPT 0x28 /* 8 ngắt sau (40..47) */
#define PIC2_END_INTERRUPT   (PIC2_START_INTERRUPT + 7) /* 47 */
#define PIC_ACK              0x20 /* Lệnh xác nhận hoàn tất ngắt */

void pic_ack(uint8_t irq);
void pic_remap(void);
void pic_set_mask(uint8_t mask1, uint8_t mask2);

#endif /* DRIVERS_PIC_H */
