#include <idt.hpp>
#include <terminal.hpp>
#include <io.h>

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

idtr_t idtr;

void IDT::SetDesc(uint8_t vector, uint32_t isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08; // this value can be whatever offset your kernel code selector is in your GDT
    descriptor->attributes     = flags;
    descriptor->isr_high       = (uint32_t)isr >> 16;
    descriptor->reserved       = 0;
}

static idt_handler_t handlers[255-31];

void IDT::AddHandler(int vector, idt_handler_t handl) {
    handlers[vector] = handl;
}

extern "C" void idt_handler(registers_t *regs) {
    if (regs->int_no > 31) {
        if (handlers[regs->int_no] != 0) {
            handlers[regs->int_no - 31](regs);
        }
        outb(0x20, 0x20);
        if (regs->int_no > 0x28) {
            outb(0xA0, 0x20);
        }
    } else {
        Terminal::Print("Oh shit, error :(\n");
        asm volatile ("cli");
        for (;;) asm volatile ("hlt");
    }
}

void idt_remap() {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

extern "C" void idt_load();
void IDT::Init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * 256 - 1;

    for (uint8_t vector = 0; vector < 255; vector++) {
        IDT::SetDesc(vector, (uint32_t)isr_common_stub, 0x8E);
    }
    idt_load();
    idt_remap();
}