#!/bin/sh
# Project Genesis - GRUB ISO creation.
#
# Wraps grub-mkrescue so the ISO layout stays reproducible:
#   iso/boot/kernel.elf
#   iso/boot/grub/grub.cfg
#
# Requires grub-mkrescue and xorriso, which are normally available on a Linux
# host (or in WSL). The build itself does not need them.
#
# Usage: sh scripts/make-iso.sh [path/to/kernel.elf]

set -u

KERNEL=${1:-build/kernel.elf}
ISO_DIR=iso
ISO_NAME=genesis.iso

for tool in grub-mkrescue xorriso; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "error: $tool not found; ISO creation is unavailable on this host." >&2
        echo "       The kernel itself still builds with 'make'." >&2
        exit 1
    fi
done

if [ ! -f "$KERNEL" ]; then
    echo "error: kernel not found at $KERNEL (run 'make' first)" >&2
    exit 1
fi

rm -rf "$ISO_DIR"
mkdir -p "$ISO_DIR/boot/grub"
cp "$KERNEL" "$ISO_DIR/boot/kernel.elf"
cp boot/grub.cfg "$ISO_DIR/boot/grub/grub.cfg"

grub-mkrescue -o "$ISO_NAME" "$ISO_DIR" || exit 1

echo "created $ISO_NAME"
