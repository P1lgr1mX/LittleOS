global loader 
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
    mov dx , 0x3D4 ; lenh chi muc khi tao bios
    mov al , 14    ; lenh chi muc khi tao bios
    out dx , al                                 ; Bật BIOS

    mov dx , 0x3D5 
    mov al , 0x00   
    out dx , al    
    
    mov dx , 0x3D4
    mov al , 15
    out dx , al                                 ; Tắt BIOS

    mov dx , 0x3D5 
    mov al , 0x50   
    out dx , al
    call kmain                                 ; Nhảy vào hàm kmain trong C

.loop: 
    hlt                                        ; Dừng CPU chờ ngắt
    jmp .loop                                  ; Vòng lặp vô hạn phòng khi CPU bị đánh thức

section .bss 
align 4 
kernel_stack: 
    resb KERNEL_STACK_SIZE                     ; Dành sẵn 4KB bộ nhớ chưa khởi tạo cho stack