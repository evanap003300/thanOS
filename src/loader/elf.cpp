#include "loader/elf.h"
#include "cpu/pmm.h"
#include "cpu/vmm.h"
#include "graphics/render.h"

namespace ELF {

static bool validate(Elf64_Ehdr* header) {
	if (header->e_ident[0] != 0x7F || header->e_ident[1] != 'E' ||
	    header->e_ident[2] != 'L'  || header->e_ident[3] != 'F') {
		terminal.printf("ELF: Bad magic\n");
		return false;
	}

	if (header->e_ident[4] != ELFCLASS64) {
		terminal.printf("ELF: Not a 64-bit binary\n");
		return false;
	}

	if (header->e_machine != EM_X86_64) {
		terminal.printf("ELF: Not an x86_64 binary\n");
		return false;
	}

	if (header->e_type != ET_EXEC) {
		terminal.printf("ELF: Not a static executable (PIE?)\n");
		return false;
	}

	return true;
}

void* load(uint8_t* image) {
	Elf64_Ehdr* header = (Elf64_Ehdr*)image;

	if (!validate(header)) {
		return nullptr;
	}

	Elf64_Phdr* phdrs = (Elf64_Phdr*)(image + header->e_phoff);

	for (uint16_t i = 0; i < header->e_phnum; i++) {
		Elf64_Phdr* ph = &phdrs[i];

		if (ph->p_type != PT_LOAD) {
			continue;
		}

		terminal.printf("ELF: Segment at %x | file %d bytes | mem %d bytes\n",
				ph->p_vaddr, ph->p_filesz, ph->p_memsz);

		// 1 + 2: allocate physical pages and map them at p_vaddr
		uint64_t first_page = ph->p_vaddr & ~0xFFFull;
		uint64_t end = (ph->p_vaddr + ph->p_memsz + 0xFFF) & ~0xFFFull;

		for (uint64_t page = first_page; page < end; page += 0x1000) {
			// Segments may share a page; only map each page once
			if (kernel_vmm.virt_to_phys((void*)page) == NULL) {
				void* phys = pmm.alloc_page();
				if (phys == NULL) {
					return nullptr;
				}
				kernel_vmm.map_memory((void*)page, phys);
			}
		}

		uint8_t* dest = (uint8_t*)ph->p_vaddr;
		uint8_t* src = image + ph->p_offset;

		// 3: copy the bytes that exist in the file
		for (uint64_t b = 0; b < ph->p_filesz; b++) {
			dest[b] = src[b];
		}

		// 4: p_memsz > p_filesz, the difference is the program's BSS
		// and the loader is who writes the zeros
		for (uint64_t b = ph->p_filesz; b < ph->p_memsz; b++) {
			dest[b] = 0;
		}
	}

	return (void*)header->e_entry;
}

}
