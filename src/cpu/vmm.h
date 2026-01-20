#pragma once
#include <stdint.h>
#include <stddef.h> 

#define PTE_PRESENT       (1ull << 0)
#define PTE_READ_WRITE    (1ull << 1)
#define PTE_USER_SUPER    (1ull << 2)
#define PTE_WRITE_THROUGH (1ull << 3)
#define PTE_CACHE_DISABLE (1ull << 4)
#define PTE_ACESSED       (1ull << 5)
#define PTE_DIRTY         (1ull << 6)
#define PTE_HUGE_PAGE     (1ull << 7)
#define PTE_GLOBAL        (1ull << 8)
#define PTE_NX            (1ull << 63)

#define PTE_ADDRESS_MASK  0x000FFFFFFFFFF000
#define PAGE_SIZE         0x1000

struct PageTable {
	uint64_t entries[512];
} __attribute__((aligned(PAGE_SIZE)));

class PageTableManager {
public:
	PageTable* pml4_address;
	
	PageTableManager(PageTable* pml4_physical_address);

	void map_memory(void* virtual_memory, void* physical_memory);
	void* virt_to_phys(void* virtual_memory);
};

extern PageTableManager kernel_vmm;
