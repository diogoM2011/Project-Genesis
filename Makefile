# Project Genesis - initial scaffold
# Build targets will be added as the bootable kernel is implemented.

.PHONY: help clean

help:
	@echo "Project Genesis"
	@echo "  make help   Show available targets"
	@echo "  make clean  Remove generated build files"

clean:
	rm -rf build
