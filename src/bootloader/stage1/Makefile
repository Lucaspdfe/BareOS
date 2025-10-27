ASM		    := nasm
ASMFLAGS    := -f bin

BUILD 		:= build
STAGE1      := $(BUILD)/stage1.bin

.PHONY: all clean

all: $(STAGE1)

$(STAGE1): boot.asm
	@$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf $(STAGE1)