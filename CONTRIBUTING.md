# Contributing

Thanks for taking an interest in Project Genesis.

## Workflow

1. Create a branch for one focused change.
2. Read the relevant design notes in `docs/`.
3. Keep changes small and explain the reasoning.
4. Build and test in QEMU where applicable.
5. Update documentation when behavior or build steps change.
6. Open a pull request describing the change and how it was tested.

## Code conventions

- Prefer clear names over clever abbreviations.
- Document non-obvious hardware and architecture details.
- Keep architecture-specific code clearly separated.
- Avoid relying on hosted C library functionality inside the kernel.
- Never test experimental boot code on a machine or disk containing important data.

## Issue reports

Include the milestone, compiler/tool versions, exact command, expected result, and actual result.
