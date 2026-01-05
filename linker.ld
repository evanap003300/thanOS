/* linker.ld */

/* Tell the linker that we want an x86_64 ELF64 output file */
OUTPUT_FORMAT(elf64-x86-64)

/* We want the symbol kmain to be our entry point */
ENTRY(kmain)

/* Define the program headers so the bootloader gives us the right permissions */
PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    /* We want to be placed in the topmost 2GiB of the address space (High Half) */
    . = 0xffffffff80000000;

    /* Define a section to contain the Limine requests */
    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    /* Move to the next memory page for .text (Code) */
    . = ALIGN(CONSTANT(MAXPAGESIZE));
    .text : {
        *(.text .text.*)
    } :text

    /* Move to the next memory page for .rodata (Read-Only Data) */
    . = ALIGN(CONSTANT(MAXPAGESIZE));
    .rodata : {
        *(.rodata .rodata.*)
        /* Build ID note needed by some linkers */
        *(.note.gnu.build-id)
    } :rodata

    /* Move to the next memory page for .data (Read-Write Data) */
    . = ALIGN(CONSTANT(MAXPAGESIZE));
    .data : {
        *(.data .data.*)
    } :data

    /* This handles C++ global constructors. Without this, global objects crash. */
    . = ALIGN(CONSTANT(MAXPAGESIZE));
    .init_array : {
        __init_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array .ctors))
        __init_array_end = .;
    } :data

    /* The BSS section (uninitialized variables) */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /* Discard unnecessary metadata sections */
    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
