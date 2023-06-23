#include "idt.hpp"
#include "terminal.hpp"

__attribute__((aligned(0x10))) 
static InterruptDescriptor32 idt[256]; // Create an array of IDT entries; aligned for performance

typedef struct {
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed)) idtr_t;
static idtr_t idtr;

void IDT::SetDesc(uint8_t vector, void* isr, uint8_t flags) {
    InterruptDescriptor32* descriptor = &idt[vector];

    descriptor->offset_1       = (uint32_t)isr & 0xFFFF;
    descriptor->selector       = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->type_attributes= flags;
    descriptor->offset_2       = (uint32_t)isr >> 16;
    descriptor->zero           = 0;
}

extern "C" void exception_handler() {
    Terminal::Print("IDT Exception!!!\n");
    for (;;) {asm volatile ("cli; hlt");}
}

extern "C" void **isr_stub_table;
void IDT::Init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(InterruptDescriptor32) * 256 - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        IDT::SetDesc(vector, isr_stub_table[vector], 0x8E);
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}