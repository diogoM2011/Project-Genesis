/*
 * Project Genesis - port I/O primitives.
 *
 * x86 uses a separate I/O address space reached through the in/out
 * instructions. These wrappers are the only place the kernel touches it.
 */

#ifndef GENESIS_IO_H
#define GENESIS_IO_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/*
 * Writing to an unused port is a cheap, well known pause on x86: it lets slow
 * legacy devices (such as the 16550 UART) catch up with a config change.
 */
static inline void io_wait(void)
{
    outb(0x80, 0);
}

static inline void cpu_halt(void)
{
    __asm__ volatile("cli; hlt");
}

#endif /* GENESIS_IO_H */
