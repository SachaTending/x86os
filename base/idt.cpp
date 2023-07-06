#include <idt.hpp>
#include <terminal.hpp>
#include <io.h>

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

static idtr_t idtr;

void IDT::SetDesc(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->attributes     = flags;
    descriptor->isr_high       = (uint32_t)isr >> 16;
    descriptor->reserved       = 0;
}

extern "C" void idt_handler(registers_t *regs) {
    if (regs->int_no > 31) {
        Terminal::Print("Interrupt!\n");
        outb(0x20, 0x20);
        if (regs->int_no > 31+7) {
            outb(0xA0, 0x20);
        }
    } else {
        Terminal::Print("Oh shit, error :(\n");
        asm volatile ("cli");
        for (;;) asm volatile ("hlt");
    }
}

void IDT::Init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * 256 - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        (vector, (void *)isr_common_stub, 0x8E);
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}