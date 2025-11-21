BUILD_DIR=build/

.PHONY: all prog2 clean
all: prog2

prog2: $(BUILD_DIR)/prog2.bin

$(BUILD_DIR)/prog2.bin: 
	nasm -f bin main.asm -o $@

clean: 
	rm -f $(BUILD_DIR)/prog2.bin