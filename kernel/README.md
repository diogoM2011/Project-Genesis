# Kernel source

The entry point and initialization path stay as small as possible. A subsystem
gets its own directory as soon as it outgrows a few lines, so this file is the
map of where things belong.

```text
kernel/
├── main.c              entry point (kmain): wires subsystems together and reports the hand-off
├── arch/
│   └── x86_64/         CPU- and platform-specific code
├── drivers/            device drivers (timer, keyboard, storage, ...)
├── mm/                 physical and virtual memory management
├── sched/              kernel threads and scheduling
├── fs/                 filesystems
└── lib/                freestanding helpers shared by the kernel (formatting, strings, ...)
```

Headers mirror this layout under `include/`, so a header path always says which
implementation it belongs to: `include/arch/x86_64/serial.h` pairs with
`kernel/arch/x86_64/serial.c`.

## Current contents

| Path | What it holds |
|---|---|
| `main.c` | `kmain`: initializes the serial console, decodes the boot hand-off, reports the result, halts |
| `arch/x86_64/serial.c` | 16550 UART driver on COM1 (115200 8N1, polling) |
| `arch/x86_64/boot_info.c` | Multiboot1/Multiboot2 decoding: bootloader name and memory sizes |
| `drivers/`, `mm/`, `sched/`, `fs/`, `lib/` | empty, kept with a `.gitkeep` until the first real file arrives |

## Rules

1. **`main.c` stays tiny.** It may initialize subsystems and print diagnostics.
   Any logic that has a name of its own belongs to a subsystem.
2. **Architecture-specific code goes under `arch/x86_64/`.** Port I/O, MSRs,
   interrupt frames, page-table formats and boot protocols are not portable; do
   not let them leak into `main.c` or `lib/`.
3. **`lib/` must stay freestanding.** No hosted C library calls, no allocation
   before the allocator exists.
4. **One subsystem per directory change.** A new directory appears in the same
   change that adds its first real source file, and the `.gitkeep` is removed then.
5. **The bootloader stub is not here.** The 32-bit entry code, page tables, GDT
   and linker script live in `boot/`, because they run before C can.
