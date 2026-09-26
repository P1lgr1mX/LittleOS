global load_idt 

section .text
load_idt: 
    mov eax, [esp + 4]              ; Retrieve pointer to struct idt_ptr from C argument
    lidt [eax]                      ; Load Interrupt Descriptor Table Register (IDTR)
    ret                             ; Return to caller
