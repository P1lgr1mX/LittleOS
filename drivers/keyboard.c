#include "drivers/keyboard.h"
#include "arch/x86/isr.h"
#include "arch/x86/io.h"
#include "drivers/framebuffer.h"
#include "drivers/serial.h"

/* Trạng thái phím Shift */
static int shift_active = 0;

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
            /* In ký tự ra màn hình Framebuffer */
            char buf[2];
            buf[0] = c;
            buf[1] = '\0';
            fb_write(buf, 1);

            /* Đồng thời gửi ký tự ra cổng Serial COM1 */
            serial_write_char(SERIAL_COM1_BASE, c);

            /* Khi người dùng gõ phím Enter (\n), tự động in dấu nhắc lệnh */
            if (c == '\n') {
                const char *prompt = "LittleOS> ";
                fb_set_color(FB_LIGHT_CYAN, FB_BLACK);
                fb_write(prompt, 10);
                fb_set_color(FB_WHITE, FB_BLACK);
                serial_write(SERIAL_COM1_BASE, prompt, 10);
            }
        }
    }
}

void keyboard_init(void)
{
    /* Đăng ký hàm xử lý ngắt bàn phím tại IRQ 1 (vector ngắt số 33 = 0x21) */
    register_interrupt_handler(33, keyboard_interrupt_handler);

    /* Đọc hết dữ liệu cũ còn tồn đọng trong bộ đệm PS/2 controller */
    while (inb(KBD_STATUS_PORT) & 0x01) {
        inb(KBD_DATA_PORT);
    }
}
