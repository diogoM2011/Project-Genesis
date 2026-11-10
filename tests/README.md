# Test plan

All early tests should run in an emulator with disposable virtual disks.

| Area | Test | Expected result |
|---|---|---|
| Boot | Start the generated ISO in QEMU | Kernel entry point is reached |
| Serial | Capture serial output | Boot diagnostics are readable |
| Exceptions | Trigger a controlled test exception | Exception name and state are reported |
| Memory | Allocate and release test pages | Allocator invariants remain valid |
| Timer | Count timer ticks | Counter advances consistently |
| Keyboard | Press keys in QEMU | Input events are received |
| Scheduler | Run two test threads | Both make progress |
| Filesystem | Read a known test file | Contents match expected bytes |

Add exact commands and results as each test becomes executable.
