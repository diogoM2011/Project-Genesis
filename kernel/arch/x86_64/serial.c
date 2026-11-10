/*
 * Project Genesis - COM1 serial console driver (16550 UART).
 *
 * The 16550 is configured for 115200 baud, 8 data bits, no parity, 1 stop bit,
 * which is the QEMU default and matches most terminal programs.
 */

#include <arch/x86_64/io.h>
#include <arch/x86_64/serial.h>

/* 16550 register offsets from the base I/O port. */
#define UART_DATA        0 /* also divisor low when DLAB = 1 */
#define UART_IER         1 /* also divisor high when DLAB = 1 */
#define UART_FIFO_CTRL   2
#define UART_LINE_CTRL   3
#define UART_MODEM_CTRL  4
#define UART_LINE_STATUS 5

#define UART_LSR_DATA_READY 0x01
#define UART_LSR_THR_EMPTY  0x20

#define UART_LCR_DLAB       0x80
#define UART_LCR_8N1        0x03

void serial_init(void)
{
    outb(SERIAL_COM1 + UART_IER, 0x00);    /* no interrupts: we poll */
    outb(SERIAL_COM1 + UART_LINE_CTRL, UART_LCR_DLAB);
    outb(SERIAL_COM1 + UART_DATA, 0x01);   /* divisor 1 -> 115200 baud */
    outb(SERIAL_COM1 + UART_IER, 0x00);
    outb(SERIAL_COM1 + UART_LINE_CTRL, UART_LCR_8N1);
    outb(SERIAL_COM1 + UART_FIFO_CTRL, 0xC7); /* enable and clear FIFOs */
    outb(SERIAL_COM1 + UART_MODEM_CTRL, 0x03);
}

void serial_putc(char c)
{
    /* A bare newline only moves the cursor down, so emit CR as well. */
    if (c == '\n') {
        serial_putc('\r');
    }

    while ((inb(SERIAL_COM1 + UART_LINE_STATUS) & UART_LSR_THR_EMPTY) == 0) {
        /* wait for the transmit buffer to drain */
    }

    outb(SERIAL_COM1 + UART_DATA, (uint8_t)c);
}

void serial_puts(const char *text)
{
    while (*text != '\0') {
        serial_putc(*text++);
    }
}

void serial_put_hex(uint64_t value)
{
    static const char digits[] = "0123456789abcdef";
    char buffer[16];
    int index = 0;

    serial_puts("0x");

    if (value == 0) {
        serial_putc('0');
        return;
    }

    while (value != 0) {
        buffer[index++] = digits[value & 0xF];
        value >>= 4;
    }

    while (index > 0) {
        serial_putc(buffer[--index]);
    }
}

void serial_put_dec(uint64_t value)
{
    char buffer[20];
    int index = 0;

    if (value == 0) {
        serial_putc('0');
        return;
    }

    while (value != 0) {
        buffer[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index > 0) {
        serial_putc(buffer[--index]);
    }
}
