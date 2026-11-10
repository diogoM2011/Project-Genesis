# Development roadmap

This roadmap is intentionally incremental. Complete and test each milestone before moving on.

## M0 — Toolchain and environment
- [ ] Identify host OS and install build dependencies.
- [ ] Set up an x86_64-elf cross-compiler.
- [ ] Verify assembler, linker, GRUB utilities, and QEMU.
- [ ] Document exact tool versions and commands.

**Done when:** the required tools are available and a reproducible toolchain check is documented.

## M1 — First boot
- [ ] Choose and document the boot protocol details.
- [ ] Add the boot configuration.
- [ ] Add the kernel entry point.
- [ ] Link a minimal freestanding kernel.
- [ ] Create a bootable ISO.
- [ ] Boot the ISO in QEMU.

**Done when:** QEMU loads the kernel and it reaches its entry point reliably.

## M2 — Debug output
- [ ] Initialize serial output.
- [ ] Add a small `kprintf`-style formatter (incrementally).
- [ ] Report boot milestones and errors.

**Done when:** boot messages can be read from QEMU's serial console.

## M3 — CPU descriptor tables
- [ ] Define and load a Global Descriptor Table (GDT), as required by the chosen boot environment.
- [ ] Define an Interrupt Descriptor Table (IDT).
- [ ] Add exception stubs and readable exception reports.

**Done when:** a deliberately triggered test exception is reported in a controlled way.

## M4 — Interrupt handling
- [ ] Define interrupt-frame structures.
- [ ] Configure the platform interrupt controller.
- [ ] Add a periodic timer interrupt.
- [ ] Track interrupt counts.

**Done when:** timer interrupts arrive and are handled without destabilizing the kernel.

## M5 — Memory management
- [ ] Read and document the bootloader memory map.
- [ ] Implement a physical page-frame allocator.
- [ ] Establish and document virtual-memory mappings.
- [ ] Add a kernel heap allocator.
- [ ] Add allocator tests.

**Done when:** allocations and frees pass repeatable tests and invalid states are detected.

## M6 — Basic devices
- [ ] Add a timer abstraction.
- [ ] Add keyboard input in the chosen emulated hardware configuration.
- [ ] Define a consistent driver interface.

**Done when:** input events can be read and logged through the kernel.

## M7 — Scheduling
- [ ] Define thread/process state.
- [ ] Implement cooperative kernel threads first.
- [ ] Add a scheduler test.
- [ ] Consider preemption only after timer handling is stable.

**Done when:** two test threads can take turns and preserve their state.

## M8 — Storage and filesystem
- [ ] Define block-device interfaces.
- [ ] Read from an emulated disk.
- [ ] Choose a simple filesystem format for learning.
- [ ] Add read-only support before write support.

**Done when:** a test file can be read from a disk image.

## M9 — User mode and system calls
- [ ] Define process address-space boundaries.
- [ ] Establish user-mode entry and return.
- [ ] Define a minimal system-call ABI.
- [ ] Validate pointers and arguments at the boundary.

**Done when:** a minimal user program can request a kernel service safely.

## M10 — Shell
- [ ] Parse input into commands and arguments.
- [ ] Add built-in commands such as `help`, `clear`, and `uptime`.
- [ ] Add useful error messages.

**Done when:** a user can interact with the kernel through a basic shell.
