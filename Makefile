# Makefile - build a FAT16 bootable 64MB HDD image

# Output files
IMAGE       := build/flp.img
STAGE1      := build/stage1.bin
STAGE2      := temp_root/stage2.bin
KERNEL      := temp_root/boot/kernel.bin
PROGRAM     := temp_root/usr/prog.bin

.PHONY: all clean

all: clean $(IMAGE)

# Build the bootloader binary
BOOTLOADER: $(STAGE1) $(STAGE2)

$(STAGE1): build
	@echo "[*] Assembling stage1..."
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath build/) --no-print-directory
	@echo "[+] Created $@."

$(STAGE2): build
	@echo "[*] Assembling stage2..."
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath build/) --no-print-directory
	@cp build/stage2.bin $@
	@echo "[+] Created $@."

$(KERNEL): build
	@echo "[*] Assembling kernel..."
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath build/) --no-print-directory
	@cp build/kernel.bin $@
	@echo "[+] Created $@."

$(PROGRAM): build
	@echo "[*] Assembling test program..."
	@$(MAKE) -C src/testing/prog BUILD_DIR=$(abspath build/) --no-print-directory
	@cp build/prog.bin $@
	@echo "[+] Created $@."

# Create the HDD image
$(IMAGE): $(STAGE1) $(STAGE2) $(KERNEL) $(PROGRAM) | build
	@echo "[*] Creating floppy disk image..."
	@dd if=/dev/zero of=$@ bs=512 count=2880

	@echo "[*] Creating FAT12 filesystem..."
	@mkfs.fat -F 12 -n "BARE OS" $@

	@echo "[*] Writing bootloader to MBR..."
	@dd if=$(STAGE1) of=$@ bs=512 count=1 conv=notrunc

	@echo "[*] Writing files from root/ to the image"
	@cp -r root/* temp_root/
	@mcopy -i $@ temp_root/* ::

	@echo "[+] HDD image ready: $@"
	@rm -rf temp_root

# Ensure build directory exists
build:
	@mkdir -p build
	@mkdir -p temp_root
	@mkdir -p temp_root/boot
	@mkdir -p temp_root/usr

# Clean build artifacts
clean:
	@rm -rf build
	@rm -rf temp_root
