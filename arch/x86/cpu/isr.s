extern interrupt_handler
global isr_stub_table

%macro no_error_interrupt_handler 1 
interrupt_handler_%1:
    push dword 0 
    push dword %1
    jmp common_interrupt_handler
%endmacro 

%macro error_code_interrupt_handler 1 
interrupt_handler_%1:
    push dword %1
    jmp common_interrupt_handler
%endmacro

section .text

common_interrupt_handler:
    ; Save all general-purpose registers matching struct registers layout:
    ; edi, esi, ebp, esp, ebx, edx, ecx, eax
    push eax 
    push ecx 
    push edx 
    push ebx 
    push esp 
    push ebp 
    push esi 
    push edi    

    ; Prepare arguments for C handler following cdecl calling convention:
    ; void interrupt_handler(struct registers *cpu, struct stack_state *stack, uint32_t interrupt)
    push dword [esp + 32]       ; arg3: interrupt vector number
    lea eax, [esp + 40]         ; arg2: pointer to struct stack_state
    push eax
    lea eax, [esp + 8]          ; arg1: pointer to struct registers
    push eax

    call interrupt_handler

    add esp, 12                 ; Clean up the 3 arguments pushed on the stack

    ; Restore general-purpose registers in reverse order
    pop edi 
    pop esi 
    pop ebp 
    add esp, 4                  ; Discard the saved ESP value
    pop ebx 
    pop edx 
    pop ecx 
    pop eax 

    ; Discard error code and interrupt vector number from the stack
    add esp, 8 
    iret

; Define 32 ISR stubs for CPU exceptions (vectors 0 - 31)
no_error_interrupt_handler 0
no_error_interrupt_handler 1
no_error_interrupt_handler 2
no_error_interrupt_handler 3
no_error_interrupt_handler 4
no_error_interrupt_handler 5
no_error_interrupt_handler 6
no_error_interrupt_handler 7
error_code_interrupt_handler 8
no_error_interrupt_handler 9
error_code_interrupt_handler 10
error_code_interrupt_handler 11
error_code_interrupt_handler 12
error_code_interrupt_handler 13
error_code_interrupt_handler 14
no_error_interrupt_handler 15
no_error_interrupt_handler 16
error_code_interrupt_handler 17
no_error_interrupt_handler 18
no_error_interrupt_handler 19
no_error_interrupt_handler 20
no_error_interrupt_handler 21
no_error_interrupt_handler 22
no_error_interrupt_handler 23
no_error_interrupt_handler 24
no_error_interrupt_handler 25
no_error_interrupt_handler 26
no_error_interrupt_handler 27
no_error_interrupt_handler 28
no_error_interrupt_handler 29
no_error_interrupt_handler 30
no_error_interrupt_handler 31

; Define 16 ISR stubs for 8259 PIC hardware IRQs (vectors 32 - 47)
no_error_interrupt_handler 32
no_error_interrupt_handler 33
no_error_interrupt_handler 34
no_error_interrupt_handler 35
no_error_interrupt_handler 36
no_error_interrupt_handler 37
no_error_interrupt_handler 38
no_error_interrupt_handler 39
no_error_interrupt_handler 40
no_error_interrupt_handler 41
no_error_interrupt_handler 42
no_error_interrupt_handler 43
no_error_interrupt_handler 44
no_error_interrupt_handler 45
no_error_interrupt_handler 46
no_error_interrupt_handler 47

; Vector dispatch table containing addresses of the 48 ISR stubs
section .data
align 4
isr_stub_table:
%assign i 0
%rep 48
    dd interrupt_handler_%+i
%assign i i+1
%endrep
