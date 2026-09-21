#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "io.h"

struct registers {
   unsigned int edi , esi , ebp , esp , ebx , edx , ecx , eax;

} __attribute__((packed));

struct stack_state{
    unsigned int error_code ; 
    unsigned int eip ;
    unsigned int cs ;
    unsigned int eflags ; 
} __attribute__((packed)); 

void interrupt_handler(struct registers *cpu, struct stack_state *stack, unsigned int interrupt);