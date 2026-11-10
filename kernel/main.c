/*
 * Project Genesis - kernel entry point.
 *
 * Reached from boot/boot.s after the CPU has been switched to 64-bit long mode.
 * For now the kernel only identifies the boot hand-off and proves it is running
 * by writing to the serial console (Milestones 1 and 2 of docs/ROADMAP.md).
 */

#include <io.h>
#include <serial.h>

/* Values the bootloader leaves in EAX, as required by each protocol. */
#define MULTIBOOT1_BOOT_MAGIC 0x2BADB002u
#define MULTIBOOT2_BOOT_MAGIC 0x36D76289u

/* Multiboot1 information structure (only the leading fields we read). */
struct multiboot1_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
} __attribute__((packed));

#define MB1_FLAG_MEMORY         (1u << 0)
#define MB1_FLAG_BOOTLOADER     (1u << 9)

/* Multiboot2 information is a size-prefixed list of tags. */
struct multiboot2_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct multiboot2_string_tag {
    struct multiboot2_tag header;
    char string[];
} __attribute__((packed));

#define MB2_TAG_END              0
#define MB2_TAG_BOOTLOADER_NAME  2

static const char *multiboot2_bootloader_name(uint32_t info_address)
{
    const uint8_t *cursor = (const uint8_t *)(uintptr_t)info_address;

    if (info_address == 0) {
        return "(none)";
    }

    /* Skip the total-size field, then walk 8-byte aligned tags. */
    uint32_t total_size = *(const uint32_t *)cursor;
    cursor += 8;

    while ((uint32_t)(cursor - (const uint8_t *)(uintptr_t)info_address) < total_size) {
        const struct multiboot2_tag *tag = (const struct multiboot2_tag *)cursor;

        if (tag->type == MB2_TAG_END) {
            break;
        }
        if (tag->size < sizeof(struct multiboot2_tag)) {
            break; /* malformed tag: stop instead of looping forever */
        }
        if (tag->type == MB2_TAG_BOOTLOADER_NAME) {
            const struct multiboot2_string_tag *string_tag =
                (const struct multiboot2_string_tag *)tag;
            return string_tag->string;
        }

        /* Tags are padded up to the next 8-byte boundary. */
        cursor += (tag->size + 7u) & ~7u;
    }

    return "(none)";
}

void kmain(uint32_t magic, uint32_t boot_info_address)
{
    serial_init();

    serial_puts("\nProject Genesis\n");
    serial_puts("===============\n");

    serial_puts("boot magic : ");
    serial_put_hex(magic);
    serial_puts("\n");

    if (magic == MULTIBOOT2_BOOT_MAGIC) {
        serial_puts("protocol   : multiboot2\n");
        serial_puts("bootloader : ");
        serial_puts(multiboot2_bootloader_name(boot_info_address));
        serial_puts("\n");
        serial_puts("info addr  : ");
        serial_put_hex(boot_info_address);
        serial_puts("\n");
    } else if (magic == MULTIBOOT1_BOOT_MAGIC) {
        const struct multiboot1_info *info =
            (const struct multiboot1_info *)(uintptr_t)boot_info_address;

        serial_puts("protocol   : multiboot1\n");

        if (info != 0 && (info->flags & MB1_FLAG_BOOTLOADER) != 0 && info->boot_loader_name != 0) {
            serial_puts("bootloader : ");
            serial_puts((const char *)(uintptr_t)info->boot_loader_name);
            serial_puts("\n");
        }

        if (info != 0 && (info->flags & MB1_FLAG_MEMORY) != 0) {
            serial_puts("memory     : ");
            serial_put_dec(info->mem_lower);
            serial_puts(" KiB below 1 MiB, ");
            serial_put_dec(info->mem_upper);
            serial_puts(" KiB above 1 MiB\n");
        }
    } else {
        serial_puts("protocol   : unknown boot protocol\n");
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
