CPP = g++
CC = gcc
LD = ld
NASM = nasm

CFLAGS = -I include -m32 -fno-leading-underscore -march=i386 -ffreestanding -O2 -Wall -Wextra -fno-rtti -fno-stack-protector -fpermissive -fno-stack-check -c -g
LDFLAGS = -melf_i386 -A i386 -T link.ld

OBJ = 

ifeq ($(OS),Windows_NT)
is_win = yes
endif
ifeq (FORCE_NOWIN,yes)
is_win = no
endif

all: kernel
-include targets/*.mk
.PHONY: kernel
kernel: $(OBJ)
ifeq (is_win,yes)
	@echo "  [LD]   kernel.exe(temporary)"
	@ld -A i386 -T link.ld -o kernel.exe -m i386pe $(OBJ)
	@echo "Generating kernel..."
	@objcopy -O elf32-i386 kernel.exe kernel.elf
	@echo "  [RM]   kernel.exe"
	@del kernel.exe

else
	@echo "  [LD]   kernel.elf"
	@$(LD) $(LDFLAGS) $(OBJ) -o kernel.elf
endif
%.o: %.cpp
	@echo "  [C++]  $@"
	@$(CPP) $(CFLAGS) -o $@ $<

%.o: %.c
	@echo "  [CC]   $@"
	@$(CC) $(CFLAGS) -o $@ $<

%.o: %.asm
	@echo "  [NASM] $@"
	@$(NASM) -felf32 -o $@ $< -g

clean:
ifeq ($(OS),Windows_NT)
	@py clear.py $(OBJ)
else
	@-rm $(OBJ)
endif
run: kernel
	@qemu-system-i386 -kernel kernel.elf -serial stdio -device vmware-svga -device rtl8139,netdev=a -netdev user,id=a,hostfwd=udp::1200-:1200 -m 512m