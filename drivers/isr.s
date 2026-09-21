%macro no_error_interrupt_handler 1 
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0 
    push dword %1
    jmp common_interrupt_handler
%endmacro 

%macro error_code_interrupt_handler 1 
global interrupt_handler_%1
interrupt_handler_%1:
    push dword %1
    jmp common_interrupt_handler
%endmacro

common_interrupt_handler:
 ; edi , esi , ebp , esp , ebx , edx , ecx , eax;
    push eax 
    push ecx 
    push edx 
    push ebx 
    push esp 
    push ebp 
    push esi 
    push edi    

    call interrupt_handler

    pop edi 
    pop esi 
    pop ebp 
    pop esp 
    pop ebx 
    pop edx 
    pop ecx 
    pop eax 
    //delete error code and interrupt number from stack
    add esp, 8 
    iret

    