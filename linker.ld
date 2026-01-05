OUTPUT_FORMAT(elf64-x86-64)

ENTRY(kmain)

SECTIONS
{
	. = 0xffffffff80000000;

	.text : { 
		*(.text*)
	}

	.rodata : {
		*(.data*)
	}

	.data : {
		*(.data*)
	}

	.init_array : {
		__init_array_stat = .;
		*(.init_array*)
		__init_array_end = .;
	}

	.bss : {
		*(COMMON)
		*(.bss*)
	}
}

