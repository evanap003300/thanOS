#pragma once
#include <stdint.h>
#include <stddef.h> 

#define PTE_PRESENT       (1ull << 0)
#define PTE_READ_WRITE    (1ull << 1)
#define PTE_USER_SUPER    (1ull << 2)
#define PTE_WRITE_THROUGH (1ull << 3)
#define PTE_CACHE_DISABLE (1ull << 4)
#define PTE_ACCESSED      (1ull << 5)
#define PTE_DIRTY         (1ull << 6)
#define PTE_HUGE_PAGE     (1ull << 7)
#define PTE_GLOBAL        (1ull << 8)
#define PTE_NX            (1ull << 63)

#define PTE_ADDRESS_MASK  0x000FFFFFFFFFF000

struct PageTable {
	uint64_t entries[512];
} __attribute__((aligned(0x1000)));

class PageTableManager {
public:
	PageTable* pml4_address;
	
	PageTableManager(PageTable* pml4_physical_address);

	void map_memory(void* virtual_memory, void* physical_memory, uint64_t flags = 0);
	void* virt_to_phys(void* virtual_memory);
};

extern PageTableManager kernel_vmm;

// Allocate a fresh PML4: empty lower half (process territory),
// kernel half shared by reference. Returns its physical address
// (i.e. a value you can load into CR3), or 0 on failure.
uint64_t create_address_space();

// The HHDM offset: phys + offset = a virtual address the kernel
// can always dereference, regardless of which CR3 is active
uint64_t get_hhdm_offset();

// Free every physical page a process owns: its leaf pages and the
// lower-half page tables, then the PML4 itself. Leaves the shared
// kernel half (entries 256-511) untouched. cr3 = the process's PML4
// physical address. MUST NOT be the currently active address space.
void free_address_space(uint64_t cr3);
