# Kernel source

The kernel implementation will live here. Keep the earliest entry point and initialization path as small as possible, then split subsystems into separate directories as they grow.

Suggested future layout:

```text
kernel/
├── arch/
│   └── x86_64/
├── drivers/
├── mm/
├── sched/
├── fs/
└── lib/
```
