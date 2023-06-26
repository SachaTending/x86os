#include "terminal.hpp"
#include "kernel.hpp"
#include "io.hpp"

int Kernel::Main(multiboot_info_t *mbinfo) 
{
    Terminal::Init();
    Terminal::Print("Hello, kernel World!\nC++ Powered\n");
    Terminal::Print("Im booted by: ");Terminal::Print((const char *)mbinfo->boot_loader_name);Terminal::Print("\n");
    return 1;
}