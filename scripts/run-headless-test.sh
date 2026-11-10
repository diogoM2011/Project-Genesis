#!/bin/sh
# Project Genesis - headless boot test.
#
# Boots the GRUB ISO in QEMU without a display, captures the serial console to a
# log file, and fails unless the expected boot diagnostics appear. This is the
# automated form of the "Boot" and "Serial" rows in tests/README.md.
#
# Usage: sh scripts/run-headless-test.sh [genesis.iso] [timeout seconds]

set -u

ISO=${1:-genesis.iso}
TIMEOUT=${2:-8}
QEMU=${QEMU:-qemu-system-x86_64}
LOG=build/serial.log

if [ ! -f "$ISO" ]; then
    echo "error: ISO not found at $ISO (run 'make iso' first)" >&2
    echo "       grub-mkrescue and xorriso are required to build it." >&2
    exit 1
fi

if ! command -v "$QEMU" >/dev/null 2>&1; then
    echo "error: $QEMU not found; run 'make check' to see what is missing" >&2
    exit 1
fi

mkdir -p build
: > "$LOG"

# -cdrom: boot the ISO through the emulated CD-ROM drive, as GRUB expects.
# -serial file: writes the guest console straight to the log.
# -display none: no window, so the test can run unattended.
# -no-reboot: a triple fault exits instead of restarting forever.
"$QEMU" -cdrom "$ISO" -m 128M \
        -display none -no-reboot -no-shutdown \
        -serial "file:$LOG" >/dev/null 2>&1 &
qemu_pid=$!

# Give the guest time to boot, then stop the emulator.
sleep "$TIMEOUT"
kill "$qemu_pid" 2>/dev/null
wait "$qemu_pid" 2>/dev/null

echo "serial output ($LOG):"
echo "---------------------"
cat "$LOG"
echo "---------------------"

status=0
check() {
    if grep -q "$1" "$LOG"; then
        echo "PASS: found '$1'"
    else
        echo "FAIL: missing '$1'"
        status=1
    fi
}

check "Project Genesis"
check "64-bit long mode"
check "result     : OK"
check "protocol   : multiboot"

if [ "$status" -eq 0 ]; then
    echo "result: boot test passed"
else
    echo "result: boot test FAILED"
fi

exit "$status"
