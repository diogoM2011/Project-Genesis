# Project Genesis

**A learning project to build a 64-bit operating-system kernel from the ground up.**

Project Genesis is an educational x86_64 kernel written in C and Assembly. The initial objective is deliberately small: build a bootable kernel, load it with GRUB, and print a message in a QEMU virtual machine. From there, the project will grow through interrupts, memory management, device drivers, multitasking, and a basic shell.

> **Status:** Milestones 1–2 scaffold — the kernel builds and links; booting in
> QEMU requires GRUB tooling (see [docs/TOOLCHAIN.md](docs/TOOLCHAIN.md))  
> **Target:** x86_64 (64-bit)  
> **Languages:** C and x86_64 Assembly  
> **Boot protocol:** GRUB Multiboot2 (planned)  
> **Test environment:** QEMU

## Project goals

- Understand the boot process and the boundary between firmware, bootloader, and kernel.
- Build a small kernel incrementally, keeping each milestone testable.
- Learn CPU architecture, memory management, interrupts, and device I/O.
- Maintain clear documentation and reproducible build instructions.

## Roadmap

| Milestone | Goal | Status |
|---|---|---|
| 0. Toolchain | Set up compiler, assembler, linker, GRUB tools, and QEMU | Partial — LLVM, make and QEMU installed; GRUB tools still missing on the Windows host |
| 1. Boot | Load a minimal kernel through GRUB | Code complete — image builds and links, boot pending GRUB tooling |
| 2. Output | Print text to a serial console | Code complete — serial driver written, output pending boot |
| 3. CPU setup | Establish GDT and IDT structures | Not started |
| 4. Interrupts | Handle exceptions and hardware interrupts | Not started |
| 5. Memory | Add physical and virtual memory management | Not started |
| 6. Devices | Add timer and keyboard support | Not started |
| 7. Scheduling | Introduce kernel threads and cooperative scheduling | Not started |
| 8. Storage | Explore block devices and a simple filesystem | Not started |
| 9. User mode | Establish a user/kernel boundary and system calls | Not started |
| 10. Shell | Build a basic interactive command-line interface | Not started |

## Repository layout

```text
Project-Genesis/
├── boot/           # Boot protocol and early entry code
├── docs/           # Design notes and learning material
├── include/        # Kernel headers
├── kernel/         # Kernel implementation
├── scripts/        # Build and run helper scripts
├── tests/          # Test plans and future automated tests
├── .gitignore
├── CONTRIBUTING.md
├── LICENSE
├── Makefile
└── README.md
```

## Development principles

1. **Small steps:** one understandable feature per change.
2. **Test in an emulator:** do not experiment on your main computer's boot disk.
3. **Document decisions:** record why an approach was chosen.
4. **Keep the kernel freestanding:** do not assume a hosted operating-system C library exists.
5. **Verify every milestone:** a feature is not complete until its test is documented.

## Building

The kernel is freestanding and built with LLVM by default (`clang` + `ld.lld`);
a GCC cross-compiler works too through `CC=`/`LD=`. QEMU boots the GRUB ISO, not
the raw ELF — QEMU's built-in Multiboot loader rejects 64-bit images.

```sh
make check   # tool versions, and what is missing
make         # build/kernel.elf
make iso     # genesis.iso (needs grub-mkrescue + xorriso)
make run     # boot the ISO in QEMU, serial console on this terminal
make test    # headless boot, verifies the serial output
```

Expected boot output over the serial console (COM1, 115200 8N1) — the kernel
builds, but this output has not been captured yet because no GRUB tooling is
installed on the development host:

```text
Project Genesis
===============
boot magic : 0x36d76289
protocol   : multiboot2
bootloader : GRUB 2.12
cpu mode   : 64-bit long mode
result     : OK - kernel entry reached and serial output works
```

See [docs/TOOLCHAIN.md](docs/TOOLCHAIN.md) for tested versions and host setup,
and [docs/ROADMAP.md](docs/ROADMAP.md) for what is done and what is next.

## How it was done

This section records how the boot-and-kernel slice was implemented and why each
decision was taken, so the next person can continue without re-deriving it.

### 1. The boot contract

`boot/boot.s` places a **Multiboot2** header as the very first bytes of the
image (it must live within the first 32 KiB of the file), followed by a
**Multiboot1** header. GRUB uses Multiboot2; the older header keeps the image
loadable by loaders that only implement the first protocol. `kmain` inspects
the magic value it receives (`0x36D76289` for Multiboot2, `0x2BADB002` for
Multiboot1) and reports which one delivered control, so the fallback is
visible rather than silent.

Both headers are kept in `KEEP(*(.multiboot))` in `boot/linker.ld`, ordered
first in the image, because the linker would otherwise drop or reorder an
unreferenced section and the loader would never find the magic.

### 2. From 32-bit to long mode

GRUB hands over in **32-bit protected mode** with paging off, so the first code
that runs is Assembly:

1. `cli` + `cld`, then save the boot magic and the boot-information pointer
   before any register is reused (they arrive in `eax` and `ebx`).
2. Build a three-level identity map: one PDPT, one PD of 512 entries with
   **2 MiB pages**, covering the first 1 GiB — enough for a kernel loaded at
   1 MiB, and cheaper than 4 KiB pages because it needs no second-level table.
3. Enable `CR4.PAE`, load `CR3` with the PML4, set `EFER.LME`, then enable
   `CR0.PG` and `CR0.WP` — this is the step that enters long mode's
   compatibility mode.
4. Load a flat 64-bit GDT and far-jump to the 64-bit code selector, which is
   what actually switches the CPU into 64-bit mode.
5. Zero `.bss`, set up a 16 KiB stack, and call `kmain`.

The page tables live in `.bss` and are cleared explicitly by the stub rather
than assumed zero: the loader is not required to zero a `NOBITS` section.

### 3. The kernel side

`kernel/main.c` is freestanding C with no dependency on a hosted library.
`kmain` validates the boot magic, walks the Multiboot information tags to find
the bootloader name and the memory map, and reports each milestone. If the
magic is wrong it says so instead of continuing with an invalid structure —
the kernel never trusts the loader blindly.

`kernel/serial.c` drives a 16550-compatible UART at **COM1 (0x3F8)**: it
disables interrupts, sets the divisor latch for 115200 baud, selects 8 data
bits / no parity / one stop bit, enables the FIFO, and then polls the
transmitter-empty bit before each byte. No interrupts are needed yet, which
keeps this milestone testable without an IDT.

### 4. Build decisions

- **LLVM instead of a GCC cross-compiler.** `clang --target=x86_64-unknown-none-elf`
  reaches the same bare-metal target with `ld.lld` and a custom linker script,
  without building a cross-toolchain. `CC=`/`LD=` still accept
  `x86_64-elf-gcc`/`x86_64-elf-ld`. Details in [docs/TOOLCHAIN.md](docs/TOOLCHAIN.md).
- **Freestanding flags that matter:** `-ffreestanding -fno-builtin`,
  `-fno-pic -fno-pie` (fixed load address), `-mno-red-zone` (interrupt
  handlers may not use it), `-mgeneral-regs-only` (no SSE state in the kernel
  yet), and `-fno-asynchronous-unwind-tables` (no `.eh_frame`).
- **Separate `ASFLAGS`.** `-O2`, `-std=gnu11` and friends are C-only; clang
  rejects them as unused arguments when assembling a `.s` file, and with
  `-Werror` that breaks the build. Assembly gets only the target and `-Iinclude`.
- **`ifeq ($(origin CC),default)`** in the Makefile: GNU make predefines
  `CC=cc`, so `CC ?= clang` silently keeps `cc`. The conditional is what
  actually selects the cross-target.
- **`SHELL := sh` on Windows**, because the default recipe shell is `cmd.exe`,
  which has no `mkdir -p` or `rm`.

### 5. How it was verified

- `make clean && make` → exit 0, produces `build/kernel.elf` (ELF64,
  `EM_X86_64`, entry `0x101000`).
- Header check on the linked file: Multiboot2 magic at offset `0x1000`,
  8-byte aligned, header checksum zero; Multiboot1 at `0x1018`, 4-byte
  aligned, checksum zero.
- `make check` → exit 0, reports tool versions and what is missing.
- `sh -n` on every script → no syntax errors.

**Not verified yet:** an actual boot. QEMU's built-in Multiboot loader rejects
64-bit images (`Cannot load x86-64 image, give a 32bit one`), so the kernel must
be booted from the GRUB ISO, and `grub-mkrescue`/`xorriso` are not available on
the current Windows host. `make test` fails with an explicit message until they
are installed (WSL or a Linux machine). That is the first task for the next
slice, before any kernel work can be called "working".

## Safety

Use QEMU or another disposable virtual machine for experiments. Do not write experimental boot images to a real disk or change your computer's bootloader while learning.

## License

See [LICENSE](LICENSE).
