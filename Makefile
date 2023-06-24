CPP = g++
LD = ld
NASM = nasm

CFLAGS = -I include -m32 -march=i386 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -c -g
LDFLAGS = -melf_i386 -A i386 -T link.ld

OBJ = 

-include targets/*.mk

all: kernel

kernel: $(OBJ)
	@echo "  [LD]   kernel.elf"
	@$(LD) $(LDFLAGS) $(OBJ) -o kernel.elf

%.o: %.cpp
	@echo "  [C++]  $@"
	@$(CPP) $(CFLAGS) -o $@ $<

%.o: %.asm
	@echo "  [NASM] $@"
	@$(NASM) -felf32 -o $@ $<

clean:
	@-rm $(OBJ)

run: kernel
	@qemu-system-i386 -kernel kernel.elf -serial stdio