/* 
 * link.ld: Kịch bản liên kết dành cho Higher-Half Kernel (AetherOS)
 *
 * Ý tưởng cốt lõi:
 * 1. Bootloader (GRUB) nạp Kernel vào RAM vật lý tại mốc 1MB (0x00100000).
 *    Do đó, Load Memory Address (LMA) của mọi section phải bắt đầu từ 0x00100000.
 * 2. Ta muốn toàn bộ mã lệnh, biến, con trỏ của Kernel được định vị và chạy
 *    trong không gian địa chỉ ảo (VMA) ở mốc 3GB + 1MB (0xC0100000).
 * 3. Điểm vào (Entry point) là `_start` nằm trong section `.boot` ở mốc 1MB,
 *    để khi GRUB nhảy vào (lúc Paging chưa bật), CPU có thể chạy mã khởi động
 *    mà không bị crash bộ nhớ.
 */

ENTRY(_start)

SECTIONS
{
    /* -------------------------------------------------------------
     * Giai đoạn 1: Phần khởi động sơ cấp (Bootstrap)
     * VMA = LMA = 0x00100000 (Địa chỉ vật lý 1MB)
     * ------------------------------------------------------------- */
    . = 0x00100000;

    .boot : {
        *(.multiboot)   /* Header Multiboot (bắt buộc nằm trong 8KB đầu) */
        *(.boot)        /* Mã Assembly khởi tạo bảng trang và bật Paging */
    }

    /* -------------------------------------------------------------
     * Giai đoạn 2: Kernel chính chuyển sang Higher-Half
     * Dời Virtual Memory Address (VMA) lên mốc 3GB: 0xC0000000
     * Load Memory Address (LMA) được tính qua chỉ thị AT(ADDR(...) - 0xC0000000)
     * ------------------------------------------------------------- */
    . += 0xC0000000;

    /* Mã lệnh Kernel (.text): căn chỉnh theo ranh giới trang 4KB */
    .text ALIGN (4096) : AT(ADDR(.text) - 0xC0000000) {
        *(.text)
    }

    /* Dữ liệu chỉ đọc (.rodata) */
    .rodata ALIGN (4096) : AT(ADDR(.rodata) - 0xC0000000) {
        *(.rodata*)
    }

    /* Dữ liệu đã khởi tạo (.data) */
    .data ALIGN (4096) : AT(ADDR(.data) - 0xC0000000) {
        *(.data)
    }

    /* Dữ liệu chưa khởi tạo (.bss) và biến dùng chung (COMMON) */
    .bss ALIGN (4096) : AT(ADDR(.bss) - 0xC0000000) {
        *(COMMON)
        *(.bss)
    }
}
