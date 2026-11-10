/*
 * Project Genesis - boot hand-off information.
 *
 * Each boot protocol delivers the same facts in a different shape: the magic
 * value left in EAX, the bootloader name, and the amount of usable memory.
 * This header is the kernel-facing view, so the rest of the kernel never has to
 * know which protocol the loader spoke.
 */

#ifndef GENESIS_ARCH_X86_64_BOOT_INFO_H
#define GENESIS_ARCH_X86_64_BOOT_INFO_H

#include <stdint.h>

/* Values the bootloader leaves in EAX, as required by each protocol. */
#define MULTIBOOT1_BOOT_MAGIC 0x2BADB002u
#define MULTIBOOT2_BOOT_MAGIC 0x36D76289u

struct boot_info {
    uint32_t magic;            /* raw magic value from the loader */
    const char *protocol;      /* "multiboot1", "multiboot2" or "unknown" */
    const char *loader;        /* bootloader name, or 0 when not provided */
    uint32_t info_address;     /* pointer the loader passed in EBX */
    uint32_t memory_lower_kib; /* KiB below 1 MiB, valid when has_memory */
    uint32_t memory_upper_kib; /* KiB above 1 MiB, valid when has_memory */
    int has_memory;            /* non-zero when the two fields above are valid */
};

/*
 * Decode the hand-off. Always fills every field; a wrong magic value yields
 * protocol "unknown" with no loader and no memory information, so the caller can
 * report the failure instead of dereferencing an invalid structure.
 */
void boot_info_parse(uint32_t magic, uint32_t info_address, struct boot_info *out);

#endif /* GENESIS_ARCH_X86_64_BOOT_INFO_H */
