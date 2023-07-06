#include <terminal.hpp>
#include <gdt.hpp>
#include <idt.hpp>

extern "C" void kernel_main() {
	Terminal::Init();
	Terminal::Print("Starting...\n");
	GDT::Init();
	Terminal::Print("GDT Initializated.\n");
	IDT::Init();
	Terminal::Print("IDT Initializated.\n");
	for (;;);
}