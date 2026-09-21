global outb 

section .text
outb:
    mov dx, [esp + 4]    ; port: địa chỉ cổng I/O (16-bit)
    mov al, [esp + 8]    ; value: dữ liệu cần gửi (8-bit)
    out dx, al           ; ghi 1 byte từ thanh ghi AL ra cổng DX
    ret                  ; trở về hàm gọi C 

global inb 
inb : 
    mov dx, [esp + 4]    ; port: địa chỉ cổng I/O (16-bit)
    in al, dx            ; nhận 1 byte từ cổng DX vào AL
    ret                  ; trở về hàm gọi C
