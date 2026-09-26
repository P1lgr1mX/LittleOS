#ifndef DRIVERS_PIC_H
#define DRIVERS_PIC_H

#include "types.h"

#define PIC1_PORT_A          0x20 /* Master PIC Command / Status port */
#define PIC1_PORT_B          0x21 /* Master PIC Data / Interrupt Mask port */
#define PIC2_PORT_A          0xA0 /* Slave PIC Command / Status port */
#define PIC2_PORT_B          0xA1 /* Slave PIC Data / Interrupt Mask port */

#define PIC1_START_INTERRUPT 0x20 /* Master PIC base vector offset (32..39) */
#define PIC2_START_INTERRUPT 0x28 /* Slave PIC base vector offset (40..47) */
#define PIC2_END_INTERRUPT   (PIC2_START_INTERRUPT + 7) /* 47 */
#define PIC_ACK              0x20 /* Non-specific End-of-Interrupt (EOI) command code */

void pic_ack(uint8_t irq);
void pic_remap(void);
void pic_set_mask(uint8_t mask1, uint8_t mask2);

#endif /* DRIVERS_PIC_H */
