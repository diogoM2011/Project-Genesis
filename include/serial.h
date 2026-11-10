/*
 * Project Genesis - serial console.
 *
 * Early diagnostics go to COM1 because a serial line is the only output channel
 * that works before any framebuffer driver exists, and because QEMU can capture
 * it directly (-serial stdio).
 */

#ifndef GENESIS_SERIAL_H
#define GENESIS_SERIAL_H

#include <stdint.h>

#define SERIAL_COM1 0x3F8

void serial_init(void);
void serial_putc(char c);
void serial_puts(const char *text);
void serial_put_hex(uint64_t value);
void serial_put_dec(uint64_t value);

#endif /* GENESIS_SERIAL_H */
