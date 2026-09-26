#ifndef ARCH_X86_ISR_H
#define ARCH_X86_ISR_H

#include "types.h"
#include "arch/x86/io.h"

/* Structure representing general-purpose CPU registers saved by common_interrupt_handler */
struct registers {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
} __attribute__((packed));

/* Structure representing stack frame pushed upon CPU interrupt/exception entry */
struct stack_state {
    uint32_t error_code; 
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags; 
} __attribute__((packed)); 

typedef void (*isr_handler_t)(struct registers *cpu, struct stack_state *stack, uint32_t interrupt);

void register_interrupt_handler(uint8_t n, isr_handler_t handler);
void interrupt_handler(struct registers *cpu, struct stack_state *stack, uint32_t interrupt);

#endif /* ARCH_X86_ISR_H */
