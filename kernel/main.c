/*
 * Project Genesis - kernel entry point.
 *
 * Reached from boot/boot.s after the CPU has been switched to 64-bit long mode.
 *
 * This file stays deliberately tiny: it wires subsystems together and reports
 * the hand-off. Anything that grows beyond a few lines moves into its own
 * subsystem directory under kernel/ (arch, drivers, mm, sched, fs, lib).
 */

#include <arch/x86_64/boot_info.h>
#include <arch/x86_64/io.h>
#include <arch/x86_64/serial.h>

static void report_boot_info(const struct boot_info *info)
{
    serial_puts("boot magic : ");
    serial_put_hex(info->magic);
    serial_puts("\n");

    serial_puts("protocol   : ");
    serial_puts(info->protocol);
    serial_puts("\n");

    if (info->loader != 0) {
        serial_puts("bootloader : ");
        serial_puts(info->loader);
        serial_puts("\n");
    }

    if (info->info_address != 0) {
        serial_puts("info addr  : ");
        serial_put_hex(info->info_address);
        serial_puts("\n");
    }

    if (info->has_memory != 0) {
        serial_puts("memory     : ");
        serial_put_dec(info->memory_lower_kib);
        serial_puts(" KiB below 1 MiB, ");
        serial_put_dec(info->memory_upper_kib);
        serial_puts(" KiB above 1 MiB\n");
    }
}

void kmain(uint32_t magic, uint32_t info_address)
{
    struct boot_info info;

    serial_init();

    serial_puts("\nProject Genesis\n");
    serial_puts("===============\n");

    boot_info_parse(magic, info_address, &info);
    report_boot_info(&info);

    if (info.magic != MULTIBOOT1_BOOT_MAGIC && info.magic != MULTIBOOT2_BOOT_MAGIC) {
        serial_puts("result     : FAILED - the boot hand-off is not Multiboot\n");
        goto halt;
    }

    serial_puts("cpu mode   : 64-bit long mode\n");
    serial_puts("result     : OK - kernel entry reached and serial output works\n");

halt:
    serial_puts("\nsystem halted.\n");

    for (;;) {
        cpu_halt();
    }
}
