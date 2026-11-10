# Project Genesis

**A learning project to build a 64-bit operating-system kernel from the ground up.**

Project Genesis is an educational x86_64 kernel written in C and Assembly. The initial objective is deliberately small: build a bootable kernel, load it with GRUB, and print a message in a QEMU virtual machine. From there, the project will grow through interrupts, memory management, device drivers, multitasking, and a basic shell.

> **Status:** Planning / initial scaffold  
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
| 0. Toolchain | Set up compiler, assembler, linker, GRUB tools, and QEMU | Not started |
| 1. Boot | Load a minimal kernel through GRUB | Not started |
| 2. Output | Print text to a serial console | Not started |
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

The initial scaffold does not yet contain a bootable kernel. The toolchain and exact build commands will be documented as Milestone 0 is completed. See [docs/ROADMAP.md](docs/ROADMAP.md) and [docs/TOOLCHAIN.md](docs/TOOLCHAIN.md).

## Safety

Use QEMU or another disposable virtual machine for experiments. Do not write experimental boot images to a real disk or change your computer's bootloader while learning.

## License

See [LICENSE](LICENSE).
