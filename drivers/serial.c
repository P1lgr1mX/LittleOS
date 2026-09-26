#include "drivers/serial.h"
#include "drivers/keyboard.h"
#include "arch/x86/io.h"
#include "arch/x86/isr.h"

#define SERIAL_COM1_BASE                0x3F8 /* COM1 base I/O port address */
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_INT_ENABLE_PORT(base)    (base + 1)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

#define SERIAL_LINE_ENABLED_DLAB        0x80

void serial_config_baud_rate(uint32_t base, uint32_t baud_rate)
{
    uint16_t divisor = (baud_rate >= 110) ? (uint16_t)(115200 / baud_rate) : (baud_rate == 0 ? 1 : (uint16_t)baud_rate);
    outb(SERIAL_LINE_COMMAND_PORT(base), SERIAL_LINE_ENABLED_DLAB);
    outb(SERIAL_DATA_PORT(base), (uint8_t)(divisor & 0xFF));
    outb(SERIAL_DATA_PORT(base) + 1, (uint8_t)((divisor >> 8) & 0xFF));
}

void serial_config_line(uint16_t com)
{
    /* 0x03: 8 data bits, 1 stop bit, no parity (8N1), DLAB disabled */
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

void serial_config_buffers(uint16_t com)
{
    /* 0x07: Enable FIFO, clear transmit/receive queues, 1-byte trigger threshold */
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0x07);
}

void serial_config_modem(uint16_t com)
{
    /* 0x0B = 0000 1011b: Bit 3 (OUT2: route UART interrupts to PIC), Bit 1 (RTS=1), Bit 0 (DTR=1) */
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x0B);
}

static void serial_interrupt_handler(struct registers *cpu, struct stack_state *stack, uint32_t interrupt)
{
    (void)cpu;
    (void)stack;
    (void)interrupt;

    while (serial_receive(SERIAL_COM1_BASE)) {
        int c = serial_read_char(SERIAL_COM1_BASE);
        if (c != -1) {
            /* Feed received serial character into the shared keyboard ring buffer */
            keyboard_put_char((char)c);
        }
    }
}

void serial_init(void)
{
    serial_config_baud_rate(SERIAL_COM1_BASE, 115200);
    serial_config_line(SERIAL_COM1_BASE);
    serial_config_buffers(SERIAL_COM1_BASE);
    serial_config_modem(SERIAL_COM1_BASE);

    /* Enable Received Data Available Interrupt (IER bit 0) */
    outb(SERIAL_INT_ENABLE_PORT(SERIAL_COM1_BASE), 0x01);

    /* Register interrupt handler for COM1 IRQ 4 (IDT interrupt vector 36 = 0x24) */
    register_interrupt_handler(36, serial_interrupt_handler);
}

/**
 * serial_is_transmit_fifo_empty:
 * Checks whether the UART transmitter holding register is empty.
 */
int serial_is_transmit_fifo_empty(uint32_t com)
{
    /* Bit 5 (0x20): Transmitter Holding Register Empty (THRE) */
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

int serial_write(uint32_t com, const char *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        while (serial_is_transmit_fifo_empty(com) == 0);
        outb(SERIAL_DATA_PORT(com), (uint8_t)buf[i]);
    }
    return (int) len;
}

int serial_write_char(uint32_t com, char c)
{
    while (serial_is_transmit_fifo_empty(com) == 0);
    outb(SERIAL_DATA_PORT(com), (uint8_t)c);
    return 1;
}

int serial_receive(uint32_t com)
{
    /* Bit 0: Data Ready indicator (1 when receive FIFO contains data) */
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 1;
}

int serial_read_char(uint32_t com)
{
    if (serial_receive(com)) {
        return (int)inb(SERIAL_DATA_PORT(com));
    }
    return -1;
}