global load_idt 

section .text
load_idt: 
    mov eax, [esp + 4]              ; Lấy con trỏ struct idt_ptr từ đối số hàm C
    lidt [eax]                      ; Nạp IDTR với con trỏ struct idt_ptr
    ret                             ; Trở về hàm gọi C
