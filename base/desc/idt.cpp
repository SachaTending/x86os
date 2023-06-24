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
    InterruptDescriptor32* descriptor = &idt[vector];

    descriptor->offset_1       = isr & 0xFFFF;
    descriptor->selector       = 0x08;
    descriptor->type_attributes= flags;
    descriptor->offset_2       = (uint32_t)(isr >> 16) & 0xFFFF;
    descriptor->zero           = 0;
}

extern "C" void exception_handler() {
    Terminal::Print("IDT Exception!!!\n");
    for (;;) {asm volatile ("cli; hlt");}
}

extern "C" void idt_handler(registers_t reg) {
    Terminal::Print("Interrupt!\n");
    outb(0x20, 0x20);
    if (reg.int_no >= 40) {
        outb(0xA0, 0x20);
    }
}
extern "C" void isr_common_stub();
extern "C" uint32_t **isr_stub_table;
void IDT::Init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(InterruptDescriptor32) * 256 - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        IDT::SetDesc(vector, (uint32_t)isr_stub_table[vector], 0x8E);
    }
    for (uint8_t vec=32;vec<255;vec++) {
        IDT::SetDesc(vec, (uint32_t)isr_common_stub, 0x8E);
    }
    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}