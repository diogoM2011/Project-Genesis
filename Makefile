# Project Genesis - build system.
#
# The kernel is freestanding: no hosted C library, no crt0, no dynamic linking.
# Defaults target LLVM (clang + ld.lld) with an x86_64 bare-metal target, which
# works on Windows, Linux and macOS without building a GCC cross-compiler.
#
# Override any tool on the command line, for example:
#   make CC=x86_64-elf-gcc LD=x86_64-elf-ld

# GNU make predefines CC=cc and LD=ld, and ?= does not override a defined variable,
# so only take over those two when they still hold make's built-in default.
ifeq ($(origin CC),default)
CC := clang
endif
ifeq ($(origin LD),default)
LD := ld.lld
endif

OBJCOPY ?= llvm-objcopy
QEMU    ?= qemu-system-x86_64
TARGET  ?= x86_64-unknown-none-elf

BUILD    := build
KERNEL   := $(BUILD)/kernel.elf
ISO_NAME := genesis.iso

CFLAGS := --target=$(TARGET) -std=gnu11 -O2 -g \
          -ffreestanding -fno-builtin -fno-stack-protector \
          -fno-pic -fno-pie -mno-red-zone -mgeneral-regs-only \
          -fno-asynchronous-unwind-tables -fno-unwind-tables \
          -Wall -Wextra -Werror -Iinclude

# Assembly only needs the target and the include path; the C-only flags above
# would be rejected as unused arguments when assembling a .s file.
ASFLAGS := --target=$(TARGET) -g -Iinclude

# On Windows the default recipe shell is cmd.exe, which has no mkdir -p or rm.
ifeq ($(OS),Windows_NT)
SHELL := sh
endif

LDFLAGS := -nostdlib -static -z max-page-size=0x1000 -T boot/linker.ld

# Source discovery is recursive up to three levels, so a subsystem can move into
# its own directory (kernel/arch/x86_64/..., kernel/mm/...) without touching
# this file. `find` is not used: make's wildcard keeps it shell-independent.
C_SOURCES   := $(wildcard kernel/*.c kernel/*/*.c kernel/*/*/*.c)
ASM_SOURCES := $(wildcard boot/*.s)
HEADERS     := $(wildcard include/*.h include/*/*.h include/*/*/*.h)

OBJECTS := $(patsubst boot/%.s,$(BUILD)/boot/%.o,$(ASM_SOURCES)) \
           $(patsubst kernel/%.c,$(BUILD)/kernel/%.o,$(C_SOURCES))

.PHONY: all kernel check run test iso clean distclean help

all: kernel

kernel: $(KERNEL)

$(BUILD)/boot/%.o: boot/%.s
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/kernel/%.o: kernel/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJECTS) boot/linker.ld
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)
	@echo "built $@"

# Verify the tools before blaming the code.
check:
	@sh scripts/check-toolchain.sh

# Boot the kernel in QEMU with the serial console attached to this terminal.
# Stop it with Ctrl-C.
#
# The kernel is booted from the GRUB ISO, not with QEMU's -kernel flag: QEMU's
# built-in Multiboot loader only accepts 32-bit images ("Cannot load x86-64
# image, give a 32bit one"), while this kernel is a 64-bit ELF. See
# docs/TOOLCHAIN.md.
run: iso
	$(QEMU) -cdrom $(ISO_NAME) -m 128M -serial stdio -display none -no-reboot

# Reproducible headless boot test: capture serial output and check the result.
test: iso
	@sh scripts/run-headless-test.sh $(ISO_NAME)

# Build a GRUB ISO. Requires grub-mkrescue and xorriso (a Linux/GRUB host).
iso: $(KERNEL)
	@sh scripts/make-iso.sh $(KERNEL)

clean:
	rm -rf $(BUILD)

distclean: clean
	rm -rf iso $(ISO_NAME)

help:
	@echo "Project Genesis"
	@echo "  make            Build build/kernel.elf"
	@echo "  make check      Report tool versions and missing tools"
	@echo "  make iso        Build a GRUB-bootable ISO (needs grub-mkrescue)"
	@echo "  make run        Boot the ISO in QEMU (serial on this terminal)"
	@echo "  make test       Headless QEMU boot, checks the serial output"
	@echo "  make clean      Remove build output"
