%macro IRQ 2
  global irq%1
  irq%1:
    cli
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

IRQ   0,    32
IRQ   1,    33
IRQ   2,    34
IRQ   3,    35
IRQ   4,    36
IRQ   5,    37
IRQ   6,    38
IRQ   7,    39
IRQ   8,    40
IRQ   9,    41
IRQ  10,    42
IRQ  11,    43
IRQ  12,    44
IRQ  13,    45
IRQ  14,    46
IRQ  15,    47

IRQ  80,    111

%assign i 0 
%rep    32 
    IRQ e%+i, i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32 
    dd irqe%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep

; C function in idt.cpp
extern irq_handler

global irq_common_stub:function irq_common_stub.end-irq_common_stub

; This is our common IRQ stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
irq_common_stub:
    pusha                    ; Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

    mov ax, ds               ; Lower 16-bits of eax = ds.
    push eax                 ; Save the data segment descriptor

    mov ax, 0x10             ; Load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    push esp    	     ; Push a pointer to the current top of stack - this becomes the registers_t* parameter.
    call irq_handler         ; Call into our C code.
    add esp, 4		     ; Remove the registers_t* parameter.

    pop ebx                  ; Reload the original data segment descriptor
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    mov ss, bx

    popa                     ; Pops edi,esi,ebp...
    add esp, 8               ; Cleans up the pushed error code and pushed ISR number
    iret                     ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
.end:

global idt_load
extern idtr
idt_load:
    lidt [idtr]
    sti
    ret