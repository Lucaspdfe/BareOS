ASM			:= nasm
ASMFLAGS    := -f bin

BUILD 		:= build
STAGE2      := $(BUILD)/stage2.bin

.PHONY: all clean

all: $(STAGE2)

$(STAGE2): main.asm
	@$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -rf $(STAGE2)