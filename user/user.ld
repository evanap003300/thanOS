OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
	/* Above the bootloader's low mappings and the kernel heap at 0x100000000 */
	. = 0x140000000;

	.text : { *(.text .text.*) }

	. = ALIGN(0x1000);
	.rodata : { *(.rodata .rodata.*) }

	. = ALIGN(0x1000);
	.data : { *(.data .data.*) }

	.bss : {
		*(.bss .bss.*)
		*(COMMON)
	}

	/DISCARD/ : {
		*(.eh_frame*)
		*(.note .note.*)
		*(.comment)
	}
}
