#include "terminal.hpp"
#include "kernel.hpp"

int Kernel::Main(multiboot_info_t *mbinfo) 
{
	/* Initialize terminal interface */
	Terminal::Init();

	/* Newline support is left as an exercise. */
	Terminal::Print("Hello, kernel World!\nC++ Powered\n");
    Terminal::Print("Im booted by: ");Terminal::Print((const char *)mbinfo->boot_loader_name);
    int i = 0/0;
    Terminal::Print((const char *)(0/0));
    return 0;
}