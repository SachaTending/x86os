#include <idt.hpp>
#include <terminal.hpp>
#include <io.h>
#include <libc.hpp>

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

idtr_t idtr;

void IDT::SetDesc(uint8_t vector, uint32_t isr, uint8_t flags) {
    vector += 32;
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

extern "C" void irq_handler(registers_t *regs) {
    if (regs->int_no > 31) {
        if (handlers[regs->int_no - 32] != 0) {
            handlers[regs->int_no - 32](regs);
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

    IDT::SetDesc(0, (uint32_t)irq0, 0x8E);
    IDT::SetDesc(1, (uint32_t)irq1, 0x8E);
    IDT::SetDesc(2, (uint32_t)irq2, 0x8E);
    IDT::SetDesc(3, (uint32_t)irq3, 0x8E);
    IDT::SetDesc(4, (uint32_t)irq4, 0x8E);
    IDT::SetDesc(5, (uint32_t)irq5, 0x8E);
    IDT::SetDesc(6, (uint32_t)irq6, 0x8E);
    IDT::SetDesc(7, (uint32_t)irq7, 0x8E);
    IDT::SetDesc(8, (uint32_t)irq8, 0x8E);
    IDT::SetDesc(9, (uint32_t)irq9, 0x8E);
    IDT::SetDesc(10, (uint32_t)irq10, 0x8E);
    IDT::SetDesc(11, (uint32_t)irq11, 0x8E);
    IDT::SetDesc(12, (uint32_t)irq12, 0x8E);
    IDT::SetDesc(13, (uint32_t)irq13, 0x8E);
    IDT::SetDesc(14, (uint32_t)irq14, 0x8E);
    IDT::SetDesc(15, (uint32_t)irq15, 0x8E);
    idt_load();
    idt_remap();
}