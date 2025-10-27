# Makefile - build a FAT16 bootable 64MB HDD image

# Output files
IMAGE       := build/flp.img
STAGE1      := build/stage1.bin
STAGE2      := temp_root/stage2.bin

# NASM options
NASM        := nasm
NASMFLAGS   := -f bin

.PHONY: all clean

all: clean $(IMAGE)

# Build the bootloader binary
BOOTLOADER: $(STAGE1) $(STAGE2)

$(STAGE1): src/stage1/boot.asm | build
	@echo "[*] Assembling stage1..."
	@$(NASM) $(NASMFLAGS) $< -o $@
	@echo "[+] Created $@."

$(STAGE2): src/stage2/main.asm | build
	@echo "[*] Assembling stage2..."
	@$(NASM) $(NASMFLAGS) $< -o $@
	@echo "[+] Created $@."

# Create the HDD image
$(IMAGE): $(STAGE1) $(STAGE2) | build
	@echo "[*] Creating floppy disk image..."
	@dd if=/dev/zero of=$@ bs=512 count=2880

	@echo "[*] Creating FAT16 filesystem..."
	@mkfs.fat -F 12 -n "BARE OS" $@

	@echo "[*] Writing bootloader to MBR..."
	@dd if=$(STAGE1) of=$@ bs=512 count=1 conv=notrunc

	@echo "[*] Writing files from root/ to the image"
	@cp root/* temp_root/
	@mcopy -i $@ temp_root/* ::

	@echo "[+] HDD image ready: $@"

# Ensure build directory exists
build:
	@mkdir -p build
	@mkdir -p temp_root

# Clean build artifacts
clean:
	@rm -rf build
	@rm -rf temp_root
