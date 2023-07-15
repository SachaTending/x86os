#include <terminal.hpp>
#include <gdt.hpp>
#include <idt.hpp>
#include <logging.hpp>
#include <timer.hpp>
#include <vmsvga.hpp>
#include <pci.hpp>
#include <rtl8139.hpp>
#include <mouse.hpp>
#include <multiboot.h>
#include <acpi.hpp>

static Logging log("Kernel");

typedef void (*constructor)();
extern constructor start_ctors;
extern constructor end_ctors;

multiboot_info_t *mbi;

void callConstructors(void)
{
    for(constructor* i = &start_ctors;i != &end_ctors; i++)
        (*i)();
}

void idle() {
	log.info("Going to idle.\n");
	for(;;) asm volatile("hlt");
}

const char * mmap_type_to_string(multiboot_uint32_t type) {
	switch (type)
	{
		case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
			return "acpi reclaimable";
		case MULTIBOOT_MEMORY_AVAILABLE:
			return "avaible";
		case MULTIBOOT_MEMORY_BADRAM:
			return "bad";
		case MULTIBOOT_MEMORY_NVS:
			return "nvs";
		case MULTIBOOT_MEMORY_RESERVED:
			return "reserved";
		default:
			return "unknown";
	}
}

void kbd_init();
void test();
extern "C" void kernel_main(multiboot_info_t *m) {
	mbi = m;
	Terminal::Init();
	callConstructors(); // Needed by logging system.
	log.info("Starting...\n");
	GDT::Init();
	log.info("GDT Initializated.\n");
	IDT::Init();
	log.info("IDT Initializated.\n");
	log.info("Im booted by: %s\n", mbi->boot_loader_name);
	log.info("Memory map length: %u\n", mbi->mmap_length);
	multiboot_memory_map_t *mmap;
	unsigned avaible_size = 0;
	for (mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
           (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
           mmap = (multiboot_memory_map_t *) ((unsigned long) mmap
                                    + mmap->size + sizeof (mmap->size))) {
        log.info (" size = 0x%x, base_addr = 0x%x%08x,"
                " length = 0x%x%08x, type = %s\n",
                (unsigned) mmap->size,
                (unsigned) (mmap->addr >> 32),
                (unsigned) (mmap->addr & 0xffffffff),
                (unsigned) (mmap->len >> 32),
                (unsigned) (mmap->len & 0xffffffff),
                mmap_type_to_string(mmap->type));
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
			avaible_size += mmap->len;
		}
	}
	if (avaible_size / 1024 > 1024) {
		if ((avaible_size / 1024) > 1024) {
			log.info("Avaible memory: %uMB\n", (avaible_size / 1024) / 1024);
		} else {
			log.info("Avaible memory: %uKB\n", avaible_size / 1024);
		}
	} else {
		log.info("Avaible memory: %u\n", avaible_size);
	}
	ACPI::Init();
	log.info("Starting drivers...\n");
	PCI::Init(); // Init pci
	Timer::Init(); // Init timer
	VMSVGA::Init(); // Init vmsvga
	RTL8139::Init(); // Init rtl8139
	kbd_init(); // Init keyboard.
	Mouse::Init(); // Init mouse
	asm volatile ("hlt");
	//test();
	log.info("Init done!\n");
	idle();
}