BITS 32
[ORG 0x00000000]

start:
    ; 1. In banner thông báo chế độ nhận chuỗi / buffer từ User Mode Ring 3
    mov eax, 4          ; SYS_WRITE
    mov ebx, 1          ; stdout
    mov ecx, msg_banner
    mov edx, msg_banner_len
    int 0x80

user_loop:
    ; In dấu nhắc lệnh Prompt của User Mode
    mov eax, 4          ; SYS_WRITE
    mov ebx, 1          ; stdout
    mov ecx, prompt
    mov edx, prompt_len
    int 0x80

    ; 2. Gọi SYS_READ để nhận NGUYÊN MỘT BUFFER CHUỖI HOÀN CHỈNH
    ; Kernel sẽ gom tất cả phím bấm cho đến khi gặp Enter (\n) hoặc \0
    mov eax, 3          ; SYS_READ
    mov ebx, 0          ; stdin
    mov ecx, user_buffer ; con trỏ buffer trong User space
    mov edx, 128        ; dung lượng tối đa của buffer (128 bytes)
    int 0x80

    ; eax chứa số lượng byte ký tự trong buffer (không tính \0)
    ; Nếu người dùng chỉ nhấn Enter (eax <= 0), lặp lại
    cmp eax, 0
    jle user_loop

    ; Lưu lại độ dài chuỗi nhận được
    mov [str_len], eax

    ; 3. In thông báo: "[User Ring 3 Received Buffer]: \""
    mov eax, 4
    mov ebx, 1
    mov ecx, msg_recv
    mov edx, msg_recv_len
    int 0x80

    ; In NGUYÊN VẸN NỘI DUNG BUFFER vừa nhận được từ SYS_READ
    mov eax, 4
    mov ebx, 1
    mov ecx, user_buffer
    mov edx, [str_len]
    int 0x80

    ; In phần kết thúc "\"" và xuống dòng
    mov eax, 4
    mov ebx, 1
    mov ecx, msg_end
    mov edx, msg_end_len
    int 0x80

    ; Lặp lại tiếp tục nhận chuỗi / buffer tiếp theo
    jmp user_loop

section .data
msg_banner:
    db 0x0A, "=================================================================", 0x0A
    db " [User Ring 3] Canonical Buffer Input Mode is ACTIVE!", 0x0A
    db " Type a full sentence and press ENTER (buffered until \n or \0):", 0x0A
    db "=================================================================", 0x0A, 0
msg_banner_len equ $ - msg_banner

prompt:
    db "UserRing3> ", 0
prompt_len equ $ - prompt

msg_recv:
    db "[User Ring 3 Received Buffer]: ", 0x22, 0
msg_recv_len equ $ - msg_recv

msg_end:
    db 0x22, 0x0A, 0x0D, 0
msg_end_len equ $ - msg_end

str_len:
    dd 0

section .bss
user_buffer:
    resb 128