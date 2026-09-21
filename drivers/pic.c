#include<io.h>
#define PIC1_PORT_A 0x20 // dia chi cong IO 
#define PIC1_PORT_B 0xA0 // dia chi cong IO

#define PIC1_START_INTERRUPT 0x20 //8 ngat dau 
#define PIC2_START_INTERRUPT 0x28 // 8 ngat sau 
#define PIC2_END_INTERRUPT PIC_2_START_INTERRUPT + 7 
#define PIC_ACK 0x20 //byte gui di chuyển

void pic_ack(unsigned char irq){
    if(irq < PIC1_START_INTERRUPT || irq > PIC2_END_INTERRUPT){
        return; 
    }
    if(irq < PIC2_START_INTERRUPT){
        outb(PIC1_PORT_A, PIC_ACK);
    }else{
        outb(PIC1_PORT_B, PIC_ACK);
    }
}