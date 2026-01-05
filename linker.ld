OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

PHDRS
{
	text    PT_LOAD;
	rodata  PT_LOAD;
	data    PT_LOAD;
	dynamic PT_DYNAMIC;
}

SECTIONS
{ 
	. = 0xffffffff80000000;

