#include "drivers/keyboard.h"
#include "arch/x86/isr.h"
#include "arch/x86/io.h"
#include "drivers/framebuffer.h"
#include "drivers/serial.h"

/* Trạng thái phím Shift */
static int shift_active = 0;

/* Bộ đệm vòng (Circular / Ring Buffer) lưu trữ các ký tự phím */
static char kbd_buffer[KBD_BUFFER_SIZE];
static uint32_t kbd_buf_head = 0; /* Vị trí đọc */
static uint32_t kbd_buf_tail = 0; /* Vị trí ghi */
static uint32_t kbd_buf_count = 0; /* Số lượng ký tự đang chờ */

/* Bảng chuyển đổi Scancode Set 1 sang ASCII thường */
static const char kbd_us_ascii[128] = {
    0,   27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', /* Tab */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, /* Left Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, /* Left Shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, /* Right Shift */
    '*',
    0, /* Left Alt */
    ' ', /* Space bar */
    0, /* Caps lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* F1 - F10 */
    0, /* Num lock */
    0, /* Scroll lock */
    0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Bảng chuyển đổi Scancode Set 1 sang ASCII khi nhấn giữ Shift */
static const char kbd_us_ascii_shift[128] = {
    0,   27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', /* Tab */
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, /* Left Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',
    0, /* Left Shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
    0, /* Right Shift */
    '*',
    0, /* Left Alt */
    ' ', /* Space bar */
    0, /* Caps lock */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* F1 - F10 */
    0, /* Num lock */
    0, /* Scroll lock */
    0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

uint8_t read_scan_code(void)
{
    return inb(KBD_DATA_PORT);
}

void keyboard_put_char(char c)
{
    if (kbd_buf_count < KBD_BUFFER_SIZE) {
        kbd_buffer[kbd_buf_tail] = c;
        kbd_buf_tail = (kbd_buf_tail + 1) % KBD_BUFFER_SIZE;
        kbd_buf_count++;
    }
}

int keyboard_has_char(void)
{
    return kbd_buf_count > 0;
}

int keyboard_getchar(void)
{
    if (kbd_buf_count == 0) {
        return -1;
    }
    char c = kbd_buffer[kbd_buf_head];
    kbd_buf_head = (kbd_buf_head + 1) % KBD_BUFFER_SIZE;
    kbd_buf_count--;
    return (int)(unsigned char)c;
}

char keyboard_read_char(void)
{
    enable_interrupts();
    while (!keyboard_has_char()) {
        __asm__ volatile("hlt");
    }
    return (char)keyboard_getchar();
}

static void keyboard_interrupt_handler(struct registers *cpu, struct stack_state *stack, uint32_t interrupt)
{
    (void)cpu;
    (void)stack;
    (void)interrupt;

    uint8_t scancode = read_scan_code();

    /* Bắt sự kiện nhấn / nhả phím Shift */
    if (scancode == 0x2A || scancode == 0x36) {
        shift_active = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_active = 0;
        return;
    }

    /* Bỏ qua các mã ngắt khi nhả phím (break code: bit 7 = 1) */
    if (scancode & 0x80) {
        return;
    }

    if (scancode < 128) {
        char c = shift_active ? kbd_us_ascii_shift[scancode] : kbd_us_ascii[scancode];
        if (c != 0) {
            /* Đưa ký tự vào Ring Buffer để Syscall SYS_READ tiêu thụ */
            keyboard_put_char(c);
        }
    }
}

void keyboard_init(void)
{
    kbd_buf_head = 0;
    kbd_buf_tail = 0;
    kbd_buf_count = 0;

    /* Đăng ký hàm xử lý ngắt bàn phím tại IRQ 1 (vector ngắt số 33 = 0x21) */
    register_interrupt_handler(33, keyboard_interrupt_handler);

    /* Đọc hết dữ liệu cũ còn tồn đọng trong bộ đệm PS/2 controller */
    while (inb(KBD_STATUS_PORT) & 0x01) {
        inb(KBD_DATA_PORT);
    }
}
