global outb 

mov al , [esp+8] ; byte du lieu truyền 
mov dx , [esp+4] ; chuyen dia chi vao thanh ghi dx 
out dx , al      ; ghi byte 
ret              ; return 