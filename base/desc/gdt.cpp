#include "gdt.hpp"

gdt_entry_t gdt[NUM_DESCRIPTORS];

gdt_ptr_t gdt_e;

extern "C" void gdt_flush(uint32_t gdt);

void GDT::Init() {
    gdt_e.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdt_e.base  = (uint32_t)&gdt;

    GDT::SetDesc(0, 0, 0, 0, 0);                // Null segment
    GDT::SetDesc(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    GDT::SetDesc(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    GDT::SetDesc(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
    GDT::SetDesc(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

    gdt_flush((uint32_t)&gdt_e);
}

void GDT::SetDesc(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entry_t * desc = &gdt[index];

    // Low 16 bits, middle 8 bits and high 8 bits of base
    desc->base_low = base & 0xFFFF;
    desc->base_middle = (base >> 16) & 0xFF;
    desc->base_high = (base >> 24 & 0xFF);

    /* Low 16 bits and high 4 bits of limit, since the high 4 bits of limits is between granularity and access, and we don't have 4 bit variable,
    low 4 bits of granularity actually represents high 4 bits of limits. It's weird, I know. */
    desc->limit_low = limit & 0xFFFF;
    desc->granularity = (limit >> 16) & 0x0F;

    desc->access = access;

    // Only need the high 4 bits of gran
    desc->granularity = desc->granularity | (gran & 0xF0);
}