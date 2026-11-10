# Architecture notes

## Initial target

Project Genesis initially targets x86_64 in an emulator. The first boot milestone will use GRUB and a documented boot protocol.

## Planned layers

1. **Boot interface:** receives control from the bootloader and validates boot information.
2. **Early kernel:** establishes a known execution environment and initializes diagnostics.
3. **CPU layer:** descriptor tables, exceptions, and interrupt dispatch.
4. **Memory layer:** physical frames, virtual address spaces, and heap allocation.
5. **Device layer:** timer, keyboard, and block-device interfaces.
6. **Kernel services:** scheduling, system calls, and filesystem services.
7. **User interface:** shell and eventually user programs.

## Design rule

Keep hardware-specific details behind small interfaces where practical. This makes it easier to test individual subsystems and to change emulator configurations without rewriting the entire kernel.
