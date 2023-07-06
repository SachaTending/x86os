#include <terminal.hpp>
#include <gdt.hpp>
#include <idt.hpp>
#include <logging.hpp>
#include <timer.hpp>
#include <vmsvga.hpp>
#include <pci.hpp>

static Logging log("Kernel");

typedef void (*constructor)();
extern constructor start_ctors;
extern constructor end_ctors;

void callConstructors(void)
{
    for(constructor* i = &start_ctors;i != &end_ctors; i++)
        (*i)();
}

void idle() {
	log.info("Going to idle.\n");
	for(;;) asm volatile("hlt");
}
void kbd_init();
extern int timer_tick;
extern "C" void kernel_main() {
	Terminal::Init();
	callConstructors(); // Needed by logging system.
	log.info("Starting...\n");
	GDT::Init();
	log.info("GDT Initializated.\n");
	IDT::Init();
	log.info("IDT Initializated.\n");
	log.info("Starting drivers...\n");
	PCI::Init(); // Init pci
	Timer::Init(); // Init timer
	VMSVGA::Init(); // Init vmsvga
	kbd_init(); // Init keyboard.
	asm volatile ("hlt");
	log.info("Init done!\n");
	idle();
}