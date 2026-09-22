global loader 
global load_gdt 
extern kmain                     ; Khai báo hàm kmain từ kmain.c

MAGIC_NUMBER    equ 0x1BADB002 
ALIGN_MODULES   equ 0x00000001   ; Yêu cầu GRUB căn chỉnh các module theo trang (4KB)
FLAGS           equ ALIGN_MODULES
CHECKSUM        equ -(MAGIC_NUMBER + FLAGS)
KERNEL_STACK_SIZE equ 4096

section .text 
align 4
    dd MAGIC_NUMBER 
    dd FLAGS 
    dd CHECKSUM  

loader: 
    mov esp, kernel_stack + KERNEL_STACK_SIZE   ; Thiết lập stack pointer (esp)
    push ebx                                   ; Đưa ebx (multiboot_info_t pointer) lên stack làm tham số cho kmain
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
    
.flush_cs:
    ret                    ; Chuyển đến trường hợp trên màn hình

section .bss 
align 4 
kernel_stack: 
    resb KERNEL_STACK_SIZE                     ; Dành sẵn 4KB bộ nhớ chưa khởi tạo cho stack