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
#include <int32.h>
#include <libc.hpp>

static Logging log("Kernel");

typedef void (*constructor)();
extern constructor start_ctors;
extern constructor end_ctors;

multiboot_info_t *mbi;
struct VbeInfoBlock {
   char     VbeSignature[4];         // == "VESA"
   uint16_t VbeVersion;              // == 0x0300 for VBE 3.0
   uint16_t OemStringPtr[2];         // isa vbeFarPtr
   uint8_t  Capabilities[4];
   uint16_t VideoModePtr[2];         // isa vbeFarPtr
   uint16_t TotalMemory;             // as # of 64KB blocks
   uint8_t  Reserved[492];
} __attribute__((packed));
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
void int32_test()
{
    int y;
    regs16_t regs;
     
    // switch to 320x200x256 graphics mode
    regs.ax = 0x0013;
    int32(0x10, &regs);
    
    // full screen with blue color (1)
    memset((char *)0xA0000, 1, (320*200));
     
    // draw horizontal line from 100,80 to 100,240 in multiple colors
    for(y = 0; y < 200; y++)
        memset((char *)0xA0000 + (y*320+80), y*2, 160);
     
    // wait for key
    regs.ax = 0x0000;
    int32(0x16, &regs);
     
    // switch to 80x25x16 text mode
    regs.ax = 0x0003;
    int32(0x10, &regs);
}

void vesa_test() {
	VbeInfoBlock *vbe = (VbeInfoBlock *)0x7c000;
	regs16_t regs;
	regs.es = seg(vbe);
	regs.di = off(vbe);
	regs.ax = 0x4f00;
	int32(0x10, &regs);
	log.info("oem: %s\n", (char *)desegment(vbe->OemStringPtr[0], vbe->OemStringPtr[1]));
	log.info("0x%x(%d) 0x%x(%d) 0x%x(%d)\n", vbe->OemStringPtr[0], vbe->OemStringPtr[0], vbe->OemStringPtr[1], vbe->OemStringPtr[1], desegment(vbe->OemStringPtr[0], vbe->OemStringPtr[1]));
	
}

void memmap_print() {
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
}

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
	//vesa_test();
	//int32_test();
	memmap_print();
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