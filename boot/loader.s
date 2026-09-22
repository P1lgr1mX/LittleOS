; =============================================================================
; boot/loader.s: Điểm nhập (Entry point) và Bootstrap Paging cho Higher-Half Kernel
; =============================================================================

global _start
global loader
global load_gdt
global load_page_directory
global enable_paging
global boot_page_directory
global boot_page_table1
extern kmain

; Các hằng số Multiboot 1
MAGIC_NUMBER        equ 0x1BADB002
ALIGN_MODULES       equ 0x00000001              ; Yêu cầu GRUB căn chỉnh module theo trang 4KB
FLAGS               equ ALIGN_MODULES
CHECKSUM            equ -(MAGIC_NUMBER + FLAGS) ; magic + flags + checksum phải bằng 0

KERNEL_STACK_SIZE   equ 4096                    ; Kích thước stack cho kernel (4KB)
KERNEL_VIRTUAL_BASE equ 0xC0000000              ; Địa chỉ ảo cơ sở của Higher-Half (3GB)

; =============================================================================
; Phân vùng .multiboot: GRUB đọc header này từ file ELF trong 8KB đầu tiên
; =============================================================================
section .multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

; =============================================================================
; Phân vùng .boot: Chạy tại địa chỉ vật lý 1MB (0x00100000) khi Paging CHƯA BẬT
; =============================================================================
section .boot
_start:
loader:
    ; 1. Nạp địa chỉ vật lý của thư mục trang (boot_page_directory) vào CR3.
    ;    Vì boot_page_directory nằm trong .data (địa chỉ ảo 0xC010xxxx),
    ;    ta trừ đi KERNEL_VIRTUAL_BASE để lấy địa chỉ vật lý thật (0x0010xxxx).
    mov eax, (boot_page_directory - KERNEL_VIRTUAL_BASE)
    mov cr3, eax

    ; 2. Kích hoạt phân trang: Bật bit 31 (PG - Paging Enable) trong thanh ghi CR0.
    ;    Lúc này CPU chuyển sang chế độ phân trang. Nhờ có ánh xạ Identity Mapping
    ;    (mục 0 trong thư mục trang), lệnh tiếp theo vẫn được thực thi bình thường.
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

    ; 3. Thực hiện bước nhảy tuyệt đối (Absolute Jump) lên địa chỉ ảo Higher-Half.
    ;    Nhãn `higher_half` nằm trong section .text (ở mốc >= 0xC0100000).
    lea eax, [higher_half]
    jmp eax

; =============================================================================
; Phân vùng .text: Mã lệnh chính của Kernel, chạy tại địa chỉ ảo Higher-Half (>= 0xC0100000)
; =============================================================================
section .text
higher_half:
    ; 4. Thiết lập con trỏ ngăn xếp (ESP) trỏ tới đỉnh vùng nhớ stack trong không gian ảo
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    ; 5. Đưa đối số ebx (con trỏ multiboot_info_t từ GRUB) lên stack theo chuẩn cdecl
    push ebx

    ; 6. Nhảy vào hàm kmain trong C (địa chỉ ảo tại 0xC010xxxx)
    call kmain

; Vòng lặp vô hạn phòng ngừa hàm kmain trả về
.loop:
    hlt
    jmp .loop

; Hàm nạp bảng phân đoạn GDT
load_gdt:
    mov eax, [esp + 4]              ; Lấy con trỏ struct gdt_descriptor từ đối số C
    lgdt [eax]                      ; Nạp GDTR

    mov ax, 0x10                    ; Data segment selector (offset 0x10 trong GDT)
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    jmp 0x08:.flush_cs              ; Nhảy liên phân đoạn để nạp Code segment selector 0x08

.flush_cs:
    ret

; Hàm nạp CR3 từ C
load_page_directory:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

; Hàm bật bit PG trong CR0 từ C
enable_paging:
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax
    ret

; =============================================================================
; Phân vùng .data: Khởi tạo bảng trang tĩnh tại thời điểm biên dịch
; =============================================================================
section .data
align 4096
boot_page_table1:
    ; Khởi tạo 1024 mục trang (PTE), mỗi trang 4KB -> Ánh xạ đúng 4MB bộ nhớ vật lý
    ; Cờ 3 (0b011): Present = 1, Read/Write = 1, Supervisor = 0
    %assign i 0
    %rep 1024
        dd (i * 4096) | 3
        %assign i i+1
    %endrep

align 4096
boot_page_directory:
    ; Mục 0 (quản lý 0MB - 4MB ảo):
    ; Ánh xạ Identity Map 0MB - 4MB ảo -> 0MB - 4MB vật lý (giúp CPU không crash khi vừa bật Paging)
    dd (boot_page_table1 - KERNEL_VIRTUAL_BASE) + 3
    times (768 - 1) dd 0

    ; Mục 768 (0x300, quản lý 3GB - 3GB+4MB ảo):
    ; Ánh xạ Higher-Half 3GB - 3GB+4MB ảo -> 0MB - 4MB vật lý (nơi kernel thực sự chạy)
    dd (boot_page_table1 - KERNEL_VIRTUAL_BASE) + 3
    times (1024 - 768 - 1) dd 0

; =============================================================================
; Phân vùng .bss: Dành sẵn bộ nhớ chưa khởi tạo cho ngăn xếp (Stack)
; =============================================================================
section .bss
align 4
kernel_stack:
    resb KERNEL_STACK_SIZE