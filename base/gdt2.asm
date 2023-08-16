
%define KERNEL_DATA_SEGMENT 0x10
%define CODE_DATA_SEGMENT 0x08

[GLOBAL gdt_flush]
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    jmp CODE_DATA_SEGMENT:flush 
flush:
    mov ax, KERNEL_DATA_SEGMENT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

; C declaration: void flush_tss(void);
global flush_tss
flush_tss:
	mov ax, (5 * 8) | 0 ; fifth 8-byte selector, symbolically OR-ed with 0 to set the RPL (requested privilege level).
	ltr ax
	ret