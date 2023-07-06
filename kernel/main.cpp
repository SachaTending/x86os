#include <terminal.hpp>
#include <gdt.hpp>
#include <idt.hpp>
#include <logging.hpp>

static Logging log("Kernel");

typedef void (*constructor)();
extern constructor start_ctors;
extern constructor end_ctors;

void callConstructors(void)
{
    for(constructor* i = &start_ctors;i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernel_main() {
	Terminal::Init();
	callConstructors(); // Needed by logging system.
	log.info("Starting...\n");
	GDT::Init();
	log.info("GDT Initializated.\n");
	IDT::Init();
	log.info("IDT Initializated.\n");
	log.info("Starting drivers...\n");
	for (;;)asm volatile("hlt");
}