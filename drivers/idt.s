global load_idt 

load_idt: 
    mov eax, [esp + 4]              ; Lấy con trỏ struct idt_descriptor từ đối số hàm C
    lidt [eax]                      ; Nạp IDTR với con trỏ struct idt_descriptor
    ret                             ; Trở về hàm gọi C

    