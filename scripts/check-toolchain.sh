#!/bin/sh
# Project Genesis - toolchain check.
#
# Prints the version of every tool the build uses and fails when a required one
# is missing. Optional tools (GRUB ISO creation) are reported but not required.
#
# Usage: sh scripts/check-toolchain.sh

set -u

missing=0

# report <label> <command> [version arguments...]
report() {
    label=$1
    command=$2
    shift 2

    if command -v "$command" >/dev/null 2>&1; then
        printf '  %-14s ' "$label"
        "$command" "$@" 2>&1 | head -n 1
    else
        printf '  %-14s MISSING (%s)\n' "$label" "$command"
        missing=$((missing + 1))
    fi
}

# report_optional <label> <command> <hint>
report_optional() {
    label=$1
    command=$2
    hint=$3

    if command -v "$command" >/dev/null 2>&1; then
        printf '  %-14s %s\n' "$label" "$(command -v "$command")"
    else
        printf '  %-14s not installed - %s\n' "$label" "$hint"
    fi
}

echo "Project Genesis - toolchain"
echo "----------------------------"
echo "host: $(uname -s) $(uname -m)"
echo
echo "required:"
report "compiler" "${CC:-clang}" --version
report "linker" "${LD:-ld.lld}" --version
report "assembler" "${CC:-clang}" --version
report "make" "${MAKE:-make}" --version

echo
echo "optional:"
report_optional "objcopy" "${OBJCOPY:-llvm-objcopy}" "needed only for flat binary output"
report_optional "emulator" "${QEMU:-qemu-system-x86_64}" "needed for 'make run' and 'make test'"
report_optional "grub-mkrescue" "grub-mkrescue" "needed only for 'make iso'"
report_optional "xorriso" "xorriso" "needed only for 'make iso'"

echo
if [ "$missing" -ne 0 ]; then
    echo "result: $missing required tool(s) missing"
    exit 1
fi

echo "result: required toolchain present"
