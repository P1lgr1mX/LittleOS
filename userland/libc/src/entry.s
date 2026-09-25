global _start
extern main
extern sys_exit

section .text.entry
align 4

_start:
    ; Gọi hàm main trong C
    call main

    ; Khi main kết thúc, gọi sys_exit với giá trị trả về trong eax
    push eax
    call sys_exit

    ; Đề phòng trường hợp sys_exit quay lại
.halt:
    hlt
    jmp .halt
