# Makefile - build a FAT16 bootable 64MB HDD image

# Output files
IMAGE       := build/hdd.img
BOOTLOADER  := build/stage1.bin

# NASM options
NASM        := nasm
NASMFLAGS   := -f bin

.PHONY: all clean

all: clean $(IMAGE)

# Build the bootloader binary
$(BOOTLOADER): src/boot.asm | build
	@echo "[*] Assembling bootloader..."
	@$(NASM) $(NASMFLAGS) $< -o $@

# Create the HDD image
$(IMAGE): $(BOOTLOADER) | build
	@echo "[*] Creating 64MB blank disk image..."
	@dd if=/dev/zero of=$@ bs=1M count=64

	@echo "[*] Creating FAT16 filesystem..."
	@mformat -i $@ ::

	@echo "[*] Writing bootloader to MBR..."
	@dd if=$(BOOTLOADER) of=$@ bs=512 count=1 conv=notrunc

	@echo "[+] HDD image ready: $@"

# Ensure build directory exists
build:
	@mkdir -p build

# Clean build artifacts
clean:
	@rm -rf build
