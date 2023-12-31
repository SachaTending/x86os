#pragma once
#include <stdint.h>
extern "C" void isr_common_stub();

typedef struct registers
{
    uint32_t ds;                  // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t     reserved;     // Set to zero
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed)) idtr_t;
typedef void (*idt_handler_t)(registers_t *);
namespace IDT
{
	void Init();
	void SetDesc(uint8_t vector, uint32_t isr, uint8_t flags);
	void AddHandler(int vector, idt_handler_t handl);
} // namespace IDT

extern "C" {
	extern void irq0 ();
	extern void irq1 ();
	extern void irq2 ();
	extern void irq3 ();
	extern void irq4 ();
	extern void irq5 ();
	extern void irq6 ();
	extern void irq7 ();
	extern void irq8 ();
	extern void irq9 ();
	extern void irq10();
	extern void irq11();
	extern void irq12();
	extern void irq13();
	extern void irq14();
	extern void irq15();
	extern void irq80();
	extern void isr255();
}