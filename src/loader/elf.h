#pragma once
#include <stdint.h>

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

#define EI_NIDENT 16

struct __attribute__((packed)) Elf64_Ehdr {
    uint8_t e_ident[EI_NIDENT];
    Elf64_Half e_type;           // Object file type (Executable, Shared Lib, etc.)
    Elf64_Half e_machine;        // Architecture
    Elf64_Word e_version;        // Object file version
    Elf64_Addr e_entry;          // Entry point virtual address
    Elf64_Off  e_phoff;          // Program header table file offset
    Elf64_Off  e_shoff;          // Section header table file offset
    Elf64_Word e_flags;          // Processor-specific flags
    Elf64_Half e_ehsize;         // ELF header size in bytes
    Elf64_Half e_phentsize;      // Program header table entry size
    Elf64_Half e_phnum;          // Program header table entry count
    Elf64_Half e_shentsize;      // Section header table entry size
    Elf64_Half e_shnum;          // Section header table entry count
    Elf64_Half e_shstrndx;       // Section header string table index
};

struct __attribute__((packed)) Elf64_Phdr {
    Elf64_Word p_type;           // Segment type (e.g., LOADABLE)
    Elf64_Word p_flags;          // Segment flags (Read, Write, Execute permissions)
    Elf64_Off  p_offset;         // Segment file offset (Where the code is in the file)
    Elf64_Addr p_vaddr;          // Segment virtual address (Where the code goes in RAM)
    Elf64_Addr p_paddr;          // Segment physical address
    Elf64_Xword p_filesz;        // Segment size in file
    Elf64_Xword p_memsz;         // Segment size in memory
    Elf64_Xword p_align;         // Segment alignment constraints
};