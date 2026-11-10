/*
 * Project Genesis - Multiboot information decoding.
 *
 * The loader hands the kernel a pointer to a protocol-specific structure. Both
 * layouts are decoded field by field here, defensively: a malformed tag list
 * ends the walk instead of being trusted, because the kernel must never loop
 * forever or read past the structure on bad input.
 */

#include <arch/x86_64/boot_info.h>

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

#define MB1_FLAG_MEMORY     (1u << 0)
#define MB1_FLAG_BOOTLOADER (1u << 9)

/* Multiboot2 information is a size-prefixed list of 8-byte aligned tags. */
struct multiboot2_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct multiboot2_basic_meminfo {
    struct multiboot2_tag header;
    uint32_t mem_lower;
    uint32_t mem_upper;
} __attribute__((packed));

#define MB2_TAG_END              0
#define MB2_TAG_BOOTLOADER_NAME  2
#define MB2_TAG_BASIC_MEMINFO    4

static void parse_multiboot1(uint32_t info_address, struct boot_info *out)
{
    const struct multiboot1_info *info =
        (const struct multiboot1_info *)(uintptr_t)info_address;

    out->protocol = "multiboot1";

    if (info_address == 0) {
        return;
    }

    if ((info->flags & MB1_FLAG_BOOTLOADER) != 0 && info->boot_loader_name != 0) {
        out->loader = (const char *)(uintptr_t)info->boot_loader_name;
    }

    if ((info->flags & MB1_FLAG_MEMORY) != 0) {
        out->memory_lower_kib = info->mem_lower;
        out->memory_upper_kib = info->mem_upper;
        out->has_memory = 1;
    }
}

static void parse_multiboot2(uint32_t info_address, struct boot_info *out)
{
    out->protocol = "multiboot2";

    if (info_address == 0) {
        return;
    }

    const uint8_t *base = (const uint8_t *)(uintptr_t)info_address;
    const uint32_t total_size = *(const uint32_t *)base;

    /* Skip the total-size and reserved fields, then walk the tag list. */
    for (const uint8_t *cursor = base + 8;
         (uint32_t)(cursor - base) + sizeof(struct multiboot2_tag) <= total_size;) {
        const struct multiboot2_tag *tag = (const struct multiboot2_tag *)cursor;

        if (tag->type == MB2_TAG_END) {
            break;
        }
        if (tag->size < sizeof(struct multiboot2_tag)) {
            break; /* malformed tag: stop instead of looping forever */
        }

        if (tag->type == MB2_TAG_BOOTLOADER_NAME) {
            out->loader = (const char *)(cursor + sizeof(struct multiboot2_tag));
        } else if (tag->type == MB2_TAG_BASIC_MEMINFO) {
            const struct multiboot2_basic_meminfo *memory =
                (const struct multiboot2_basic_meminfo *)cursor;

            out->memory_lower_kib = memory->mem_lower;
            out->memory_upper_kib = memory->mem_upper;
            out->has_memory = 1;
        }

        /* Tags are padded up to the next 8-byte boundary. */
        cursor += (tag->size + 7u) & ~7u;
    }
}

void boot_info_parse(uint32_t magic, uint32_t info_address, struct boot_info *out)
{
    out->magic = magic;
    out->protocol = "unknown";
    out->loader = 0;
    out->info_address = info_address;
    out->memory_lower_kib = 0;
    out->memory_upper_kib = 0;
    out->has_memory = 0;

    if (magic == MULTIBOOT2_BOOT_MAGIC) {
        parse_multiboot2(info_address, out);
    } else if (magic == MULTIBOOT1_BOOT_MAGIC) {
        parse_multiboot1(info_address, out);
    }
}
