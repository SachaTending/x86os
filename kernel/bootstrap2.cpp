#include "kernel.hpp"
#include "gdt.hpp"
#include "idt.hpp"

extern "C" void bootstrap(multiboot_info_t *mbinfo) {
    GDT::Init();
    IDT::Init();
    int ret = Kernel::Main(mbinfo);
    if (ret == 0) {
        for(;;) {
            asm volatile ("hlt");
        } // Just hang
    }
    else {
        return; // Exit to bootstrap.asm
    }
}