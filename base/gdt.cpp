#include <gdt.hpp>
#include <libc.hpp>
// Yes, this is old GDT subsystem
// But it works
gdt_entry_t gdt[NUM_DESCRIPTORS];

gdt_ptr_t gdt_info;

extern "C" void gdt_flush(uint32_t gdt);

struct tss_entry_struct {
	uint32_t prev_tss; // The previous TSS - with hardware task switching these form a kind of backward linked list.
	uint32_t esp0;     // The stack pointer to load when changing to kernel mode.
	uint32_t ss0;      // The stack segment to load when changing to kernel mode.
	// Everything below here is unused.
	uint32_t esp1; // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} __packed;
 
typedef struct tss_entry_struct tss_entry_t;

tss_entry_t tss_entry;
#define SEGMENT 0x10
void write_tss() {
	// Compute the base and limit of the TSS for use in the GDT entry.
	uint32_t base = (uint32_t) &tss_entry;
	uint32_t limit = sizeof tss_entry;

	// Add a TSS descriptor to the GDT.
	GDT::SetDesc(5, base, base + limit, 0x89, 0);
 
	// Ensure the TSS is initially zero'd.
	memset(&tss_entry, 0, sizeof tss_entry);
 
	tss_entry.ss0  = 0x10;  // Set the kernel stack segment.
	tss_entry.esp0 = 0; // Set the kernel stack pointer.
#if 0
    tss_entry.ds = SEGMENT;
    tss_entry.es = SEGMENT;
    tss_entry.fs = SEGMENT;
    tss_entry.gs = SEGMENT;
    tss_entry.ss = SEGMENT;
#endif
	//note that CS is loaded from the IDT entry and should be the regular kernel code segment
}
 
void set_kernel_stack(uint32_t stack) { // Used when an interrupt occurs
	tss_entry.esp0 = stack;
}
extern "C" void flush_tss(void);

void setup_tss() {
    write_tss();
    uint32_t esp;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    set_kernel_stack(esp);
	gdt_flush((uint32_t)&gdt_info);
	printf("gdt flushed\n");
    flush_tss();
    printf("tss flushed\n");
}

void GDT::Init() {
    gdt_info.limit = (sizeof(gdt_entry_t) * 6) - 1;
    gdt_info.base  = (uint32_t)&gdt;

    GDT::SetDesc(0, 0, 0, 0, 0);                // Null segment
    GDT::SetDesc(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
    GDT::SetDesc(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
    GDT::SetDesc(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
    GDT::SetDesc(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

    gdt_flush((uint32_t)&gdt_info);
	setup_tss();
}

void GDT::SetDesc(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entry_t * desc = &gdt[index];

    // Low 16 bits, middle 8 bits and high 8 bits of base
    desc->base_low = base & 0xFFFF;
    desc->base_middle = (base >> 16) & 0xFF;
    desc->base_high = (base >> 24 & 0xFF);
	printf("base: 0x%x 0x%x 0x%x\n", desc->base_low, desc->base_middle, desc->base_high);
    /* Low 16 bits and high 4 bits of limit, since the high 4 bits of limits is between granularity and access, and we don't have 4 bit variable,
    low 4 bits of granularity actually represents high 4 bits of limits. It's weird, I know. */
    desc->limit_low = limit & 0xFFFF;
	printf("limit low: 0x%x\n", desc->limit_low);
    desc->granularity = (limit >> 16) & 0x0F;

    desc->access = access;

    // Only need the high 4 bits of gran
    desc->granularity = desc->granularity | (gran & 0xF0);
}