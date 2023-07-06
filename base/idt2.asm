extern idt_handler
global isr_common_stub
; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
isr_common_stub:
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
    call idt_handler         ; Call into our C code.
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