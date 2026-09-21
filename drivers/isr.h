#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "io.h"

struct registers {
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
} __attribute__((packed));

struct stack_state {
    unsigned int error_code; 
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags; 
} __attribute__((packed)); 

typedef void (*isr_handler_t)(struct registers *cpu, struct stack_state *stack, unsigned int interrupt);

void register_interrupt_handler(unsigned char n, isr_handler_t handler);
void interrupt_handler(struct registers *cpu, struct stack_state *stack, unsigned int interrupt);

#endif /* INTERRUPT_H */