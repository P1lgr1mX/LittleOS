global loader 
global load_gdt 
extern kmain                     ; Khai báo hàm kmain từ kmain.c

MAGIC_NUMBER equ 0x1BADB002 
FLAGS        equ 0x0
CHECKSUM     equ -MAGIC_NUMBER 
KERNEL_STACK_SIZE equ 4096

section .text 
align 4
    dd MAGIC_NUMBER 
    dd FLAGS 
    dd CHECKSUM 

loader: 
    mov esp, kernel_stack + KERNEL_STACK_SIZE   ; Thiết lập stack pointer (esp)
    call kmain                                 ; Nhảy vào hàm kmain trong C

.loop: 
    hlt                                        ; Dừng CPU chờ ngắt
    jmp .loop                                  ; Vòng lặp vô hạn phòng khi CPU bị đánh thức

load_gdt: 
    mov eax, [esp + 4]              ; Lấy con trỏ struct gdt_descriptor từ đối số hàm C
    lgdt [eax]                      ; Nạp GDTR với con trỏ struct gdt_descriptor

    mov ax, 0x10                    ; Data segment selector (offset 0x10)
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    jmp 0x08:.flush_cs
    
.flush.cs:
    ret                    ; Chuyển đến trường hợp trên màn hình
section .bss 
align 4 
kernel_stack: 
    resb KERNEL_STACK_SIZE                     ; Dành sẵn 4KB bộ nhớ chưa khởi tạo cho stack