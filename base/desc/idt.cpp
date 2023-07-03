#include "idt.hpp"
#include "terminal.hpp"
#include "io.hpp"

__attribute__((aligned(0x10))) 
static InterruptDescriptor32 idt[256]; // Create an array of IDT entries; aligned for performance

typedef struct {
    uint16_t	limit;
    uint32_t	base;
} __attribute__((packed)) idtr_t;
static idtr_t idtr;

void IDT::SetDesc(uint8_t vector, uint32_t isr, uint8_t flags) {
    idt[vector].offset_1       = isr & 0xFFFF;
    idt[vector].selector       = 0x08;
    idt[vector].type_attributes= flags;
    idt[vector].offset_2       = (isr >> 16) & 0xFFFF;
    idt[vector].zero           = 0;
}

extern "C" void exception_handler() {
    Terminal::Print("IDT Exception!!!\n");
    for (;;) {asm volatile ("cli; hlt");}
}

extern "C" void idt_handler(registers_t *reg) {
    if (reg->int_no < 32) {
        Terminal::Print("Error!!!\n");
        asm volatile ("cli");
        for (;;) asm volatile ("hlt");
    }
    Terminal::Print("Interrupt!\n");
    if (reg->int_no-31 == 8) {
        outb(0x70, 0x0C);	// select register C
        inb(0x71);		// just throw away contents
    }
    outb(0x20, 0x20);
    if (reg->int_no >= 40) {
        outb(0xA0, 0x20);
    }
}
extern "C" void isr_common_stub();
extern "C" uint32_t **isr_stub_table;
void IDT::Init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(InterruptDescriptor32) * 256 - 1;

    for (uint8_t vector = 0; vector < 31; vector++) {
        IDT::SetDesc(vector, (uint32_t)isr_common_stub, 0x8E);
    }
    for (uint8_t vec=32;vec<32+16;vec++) {
        IDT::SetDesc(vec, (uint32_t)isr_common_stub, 0x8E);
    }
    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}