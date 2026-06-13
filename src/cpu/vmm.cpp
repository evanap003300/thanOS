#include "cpu/vmm.h"
#include "cpu/pmm.h"
#include "limine.h"

extern volatile struct limine_hhdm_request hhdm_request;

PageTableManager kernel_vmm = PageTableManager(NULL);

PageTableManager::PageTableManager(PageTable* pml4_physical_address) {
		if (hhdm_request.response != NULL) {
			uint64_t offset = hhdm_request.response->offset;
			this->pml4_address = (PageTable*)((uint64_t)pml4_physical_address + offset);
		}
}

void PageTableManager::map_memory(void* virtual_memory, void* physical_memory, uint64_t flags) {
	uint64_t virt = (uint64_t)virtual_memory;
	uint64_t phys = (uint64_t)physical_memory;
	// The user bit must be set at every level of the walk,
	// not just the leaf, or ring 3 can't reach the page
	uint64_t table_flags = PTE_PRESENT | PTE_READ_WRITE | (flags & PTE_USER_SUPER);
        uint64_t offset = hhdm_request.response->offset;
	
	uint64_t pml4_index = (virt >> 39) & 0x1FF;
        uint64_t pdp_index  = (virt >> 30) & 0x1FF;
        uint64_t pd_index   = (virt >> 21) & 0x1FF;
        uint64_t pt_index   = (virt >> 12) & 0x1FF;

	PageTable* table = this->pml4_address;

	uint64_t entry = table->entries[pml4_index];
	PageTable* next_table;

	if (!(entry & PTE_PRESENT)) {
		void* new_page = pmm.alloc_page();
		uint64_t* ptr = (uint64_t*)((uint64_t)new_page + offset);
		
		for (int i = 0; i < 512; i++) ptr[i] = 0;

		table->entries[pml4_index] = (uint64_t)new_page | table_flags;
		next_table = (PageTable*)ptr;
	} else {
		table->entries[pml4_index] |= (flags & PTE_USER_SUPER);
		uint64_t phys_addr = entry & PTE_ADDRESS_MASK;
		next_table = (PageTable*)(phys_addr + offset);
	}

	table = next_table;

	entry = table->entries[pdp_index];

	if (!(entry & PTE_PRESENT)) {
		void* new_page = pmm.alloc_page();
		uint64_t* ptr = (uint64_t*)((uint64_t)new_page + offset);
		
		for (int i = 0; i < 512; i++) ptr[i] = 0;

		table->entries[pdp_index] = (uint64_t)new_page | table_flags;
		next_table = (PageTable*)ptr;
	} else {
		table->entries[pdp_index] |= (flags & PTE_USER_SUPER);
		uint64_t phys_addr = entry & PTE_ADDRESS_MASK;
		next_table = (PageTable*)(phys_addr + offset);
	}
	table = next_table;

	entry = table->entries[pd_index];

	if (!(entry & PTE_PRESENT)) {
		void* new_page = pmm.alloc_page();
		uint64_t* ptr = (uint64_t*)((uint64_t)new_page + offset);

		for (int i = 0; i < 512; i++) ptr[i] = 0;

		table->entries[pd_index] = (uint64_t)new_page | table_flags;
		next_table = (PageTable*)ptr;
	} else {
		table->entries[pd_index] |= (flags & PTE_USER_SUPER);
		uint64_t phys_addr = entry & PTE_ADDRESS_MASK;
		next_table = (PageTable*)(phys_addr + offset);
	}
	table = next_table;

	table->entries[pt_index] = phys | PTE_PRESENT | PTE_READ_WRITE | flags;

	__asm__ volatile ("invlpg (%0)" : : "r" (virtual_memory) : "memory");
}

uint64_t get_hhdm_offset() {
	return hhdm_request.response->offset;
}

void free_address_space(uint64_t cr3) {
	uint64_t offset = hhdm_request.response->offset;
	uint64_t* pml4 = (uint64_t*)(cr3 + offset);

	// Lower half only (0-255): the process's private memory. The
	// upper half points at shared kernel tables - never free those.
	for (int i = 0; i < 256; i++) {
		if (!(pml4[i] & PTE_PRESENT)) continue;
		uint64_t pdpt_phys = pml4[i] & PTE_ADDRESS_MASK;
		uint64_t* pdpt = (uint64_t*)(pdpt_phys + offset);

		for (int j = 0; j < 512; j++) {
			if (!(pdpt[j] & PTE_PRESENT)) continue;
			uint64_t pd_phys = pdpt[j] & PTE_ADDRESS_MASK;
			uint64_t* pd = (uint64_t*)(pd_phys + offset);

			for (int k = 0; k < 512; k++) {
				if (!(pd[k] & PTE_PRESENT)) continue;
				uint64_t pt_phys = pd[k] & PTE_ADDRESS_MASK;
				uint64_t* pt = (uint64_t*)(pt_phys + offset);

				// Free the leaf data pages before the table that lists them
				for (int l = 0; l < 512; l++) {
					if (!(pt[l] & PTE_PRESENT)) continue;
					pmm.free_page((void*)(pt[l] & PTE_ADDRESS_MASK));
				}
				pmm.free_page((void*)pt_phys);
			}
			pmm.free_page((void*)pd_phys);
		}
		pmm.free_page((void*)pdpt_phys);
	}

	// Finally the root table itself
	pmm.free_page((void*)cr3);
}

uint64_t create_address_space() {
	void* phys = pmm.alloc_page();
	if (phys == NULL) {
		return 0;
	}

	uint64_t offset = hhdm_request.response->offset;
	uint64_t* pml4 = (uint64_t*)((uint64_t)phys + offset);
	uint64_t* kernel_pml4 = (uint64_t*)kernel_vmm.pml4_address;

	// Lower half (0-255) belongs to the process. Higher half
	// (256-511) points at the kernel's sub-tables, so every
	// space sees the same kernel and stays in sync.
	for (int i = 0; i < 256; i++) {
		pml4[i] = 0;
	}
	for (int i = 256; i < 512; i++) {
		pml4[i] = kernel_pml4[i];
	}

	return (uint64_t)phys;
}

void* PageTableManager::virt_to_phys(void* virtual_memory) {
	uint64_t virt = (uint64_t)virtual_memory;
	uint64_t offset = hhdm_request.response->offset;

	uint64_t pml4_index = (virt >> 39) & 0x1FF;
	uint64_t pdp_index  = (virt >> 30) & 0x1FF;
	uint64_t pd_index   = (virt >> 21) & 0x1FF;
	uint64_t pt_index   = (virt >> 12) & 0x1FF;

	PageTable* table = this->pml4_address; 

	if (!(table->entries[pml4_index] & PTE_PRESENT)) return NULL;
	table = (PageTable*)((table->entries[pml4_index] & PTE_ADDRESS_MASK) + offset);

	if (!(table->entries[pdp_index] & PTE_PRESENT)) return NULL;
	table = (PageTable*)((table->entries[pdp_index] & PTE_ADDRESS_MASK) + offset);

	if (!(table->entries[pd_index] & PTE_PRESENT)) return NULL;
	table = (PageTable*)((table->entries[pd_index] & PTE_ADDRESS_MASK) + offset);

	if (!(table->entries[pt_index] & PTE_PRESENT)) return NULL;
	
	return (void*)((table->entries[pt_index] & PTE_ADDRESS_MASK) + (virt & 0xFFF));
}
