/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)
 
/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 1M;
	kernel_start = .;
	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text BLOCK(4K) + kernel_start : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}
 
	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{		
		start_ctors = .;
		KEEP(*( .ctors ));
    	KEEP(*( .init_array ));
    	KEEP(*(SORT_BY_INIT_PRIORITY( .init_array.* )));
    	end_ctors = .;
		*(.data)
		*(.rodata) /* i know this is strange when read only data is in .data */
	}

	.modules BLOCK(4K) : ALIGN(4K)
	{
		mod_start = .;
		KEEP(*( .modules ));
    	KEEP(*(SORT_BY_INIT_PRIORITY( .modules.* )));
    	mod_end = .;
	}
	.strtab BLOCK(4K) : ALIGN(4K)
	{
		str_start = .;
		*(.strtab)
		*(.symtab)
		str_end = .;
	}
	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
	kernel_end = .;
	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}