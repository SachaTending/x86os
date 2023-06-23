#pragma once
#include "stdint.h"

struct InterruptDescriptor32 {
    uint16_t offset_1;        // offset bits 0..15
    uint16_t selector;        // a code segment selector in GDT or LDT
    uint8_t  zero;            // unused, set to 0
    uint8_t  type_attributes; // gate type, dpl, and p fields
    uint16_t offset_2;        // offset bits 16..31
};

namespace IDT
{
    void SetDesc(uint8_t vector, void* isr, uint8_t flags);
    void Init();
} // namespace IDT
