%define ENOSYS 38

global Int128Handler
extern do_syscall_in_C

section .text
align 4

Int128Handler:
    ; Khi ngắt mềm (int 0x80) kích hoạt từ Ring 3, phần cứng CPU tự động push:
    ;   [esp + 16]: ss (Ring 3)
    ;   [esp + 12]: esp (Ring 3)
    ;   [esp + 8]:  eflags
    ;   [esp + 4]:  cs (0x1B)
    ;   [esp + 0]:  eip

    ; Đẩy các thanh ghi theo đúng thứ tự cấu trúc struct pt_regs trong syscall.h:
    push eax            ; [esp + 44]: Số hiệu syscall truyền từ User Mode
    push gs             ; [esp + 40]
    push fs             ; [esp + 36]
    push es             ; [esp + 32]
    push ds             ; [esp + 28] ; dinh stack moi 
    push dword -ENOSYS  ; [esp + 24]: orig_eax (mã lỗi mặc định nếu chưa xử lý)
    push ebp            ; [esp + 20] ; tham chieu den stack  
    push edi            ; [esp + 16] ; 1 trong 5 thanh ghi ring 3 được đẩy
    push esi            ; [esp + 12]
    push edx            ; [esp + 8]
    push ecx            ; [esp + 4]
    push ebx            ; [esp + 0]

    ; Chuyển ds, es sang Kernel Data Segment (0x10) để mã C kernel truy cập bộ nhớ an toàn
    mov ax, 0x10
    mov ds, ax
    mov es, ax

    cld                 ; Đảm bảo cờ hướng (Direction Flag) luôn là chiều tăng

    ; Truyền con trỏ struct pt_regs *regs vào hàm C
    push esp
    call do_syscall_in_C
    add esp, 4          ; Dọn tham số con trỏ trên stack

    ; Khôi phục các thanh ghi tổng quát (LIFO)
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp

    ; Bỏ qua ô nhớ orig_eax (-ENOSYS) trên stack
    add esp, 4

    ; Khôi phục các thanh ghi đoạn của User
    pop ds
    pop es
    pop fs
    pop gs

    ; Khôi phục eax (chứa giá trị trả về của syscall được ghi trong regs->eax)
    pop eax

    ; Trả về User Mode (Ring 3), CPU tự động pop eip, cs, eflags, esp, ss
    iret
