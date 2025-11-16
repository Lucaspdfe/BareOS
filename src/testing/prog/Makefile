BUILD_DIR=build/

.PHONY: all prog clean
all: prog

prog: $(BUILD_DIR)/prog.bin

$(BUILD_DIR)/prog.bin: 
	nasm -f bin main.asm -o $@

clean: 
	rm -f $(BUILD_DIR)/prog.bin