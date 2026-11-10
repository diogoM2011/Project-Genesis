# Toolchain notes

The exact installation commands depend on the development host. Record the tested versions here.

## Planned tools

- **Cross-compiler:** `x86_64-elf-gcc` (or a carefully configured equivalent)
- **Assembler:** GNU assembler or NASM, depending on the selected assembly syntax
- **Linker:** cross GNU `ld`
- **Bootloader utilities:** GRUB tools supporting the chosen boot protocol
- **Emulator:** QEMU x86_64 system emulator
- **Build:** GNU Make
- **Version control:** Git

## Why a cross-compiler?

A normal host compiler targets the host operating system's ABI and runtime assumptions. A kernel is freestanding software with its own entry point, memory layout, and runtime. A cross-compiler helps avoid accidentally depending on the host environment.

## Host setup checklist

- [ ] Confirm host operating system and CPU architecture.
- [ ] Install QEMU and confirm it can start an x86_64 virtual machine.
- [ ] Install GRUB image-generation utilities.
- [ ] Build or install a suitable cross-compiler.
- [ ] Confirm the compiler targets x86_64-elf.
- [ ] Save tool versions and tested commands in this file.

Do not copy build commands from a different distribution without checking package names and versions.
