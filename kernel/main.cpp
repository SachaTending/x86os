#include "terminal.hpp"
#include "kernel.hpp"
#include "io.hpp"

int Kernel::Main(multiboot_info_t *mbinfo) 
{
    /* Initialize terminal interface */
    Terminal::Init();

    /* Newline support is left as an exercise. */
    Terminal::Print("Hello, kernel World!\nC++ Powered\n");
    Terminal::Print("Im booted by: ");Terminal::Print((const char *)mbinfo->boot_loader_name);
    outb(0x43, 0x36);
    outb(0x40, 0xFF);
    outb(0x40, 0xFF);
    asm volatile("sti");
    return 1;
}