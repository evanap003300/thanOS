#include "cpu/pmm.h"
#include "utils/io.h"
#include "graphics/render.h"
#include "limine.h"

PMM pmm;

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
        .id = LIMINE_MEMMAP_REQUEST_ID,
        .revision = 0,
	.response = NULL
};

__attribute__((used, section(".limine_requests")))
volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

void PMM::bitmap_set(uint64_t bit) {
	bitmap[bit / 8] |= (1 << (bit % 8));
}

void PMM::bitmap_unset(uint64_t bit) {
	bitmap[bit / 8] &= ~(1 << (bit % 8));
}

bool PMM::bitmap_test(uint64_t bit) {
	return bitmap[bit / 8] & (1 << (bit % 8));
}


void PMM::init() {
	if (memmap_request.response == NULL || hhdm_request.response == NULL) {
		terminal.printf("Error: Limine request failed\n");
		while (1) asm("hlt");
	}


        struct limine_memmap_response *memmap_response = memmap_request.response;
	struct limine_hhdm_response *hhdm_response = hhdm_request.response;
	
	uint64_t hhdm_offset = hhdm_response->offset;
	uint64_t highest_address = 0;

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];
		if (entry->type != (uint32_t)LIMINE_MEMMAP_USABLE && entry->type != (uint32_t)LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
		continue;
		}

		if (entry->base + entry->length > highest_address) {
			highest_address = entry->base + entry->length;
		}	
	}

	total_ram_pages = highest_address / PAGE_SIZE;
	bitmap_size = total_ram_pages / 8;

	terminal.printf("PMM: Detected %d MB RAM\n", highest_address / 1024 / 1024);
	terminal.printf("PMM: Bitmap needs %d bytes\n", bitmap_size);

	void* bitmap_phys_addr = NULL;

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];

		if (entry->type == LIMINE_MEMMAP_USABLE) {
			if (entry->length >= bitmap_size) {
				bitmap_phys_addr = (void*)entry->base;
				break;
			}
		}
	}

	if (bitmap_phys_addr == NULL) {
		terminal.printf("CRITICAL: Not enough RAM for bitmap!\n");
		while(1) asm("hlt");
	}

	bitmap = (uint8_t*)((uint64_t)bitmap_phys_addr + hhdm_offset);

	for (uint64_t i = 0; i < bitmap_size; i++) {
		bitmap[i] = 0xFF;
	}

	for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
		struct limine_memmap_entry* entry = memmap_response->entries[i];

		if (entry->type == LIMINE_MEMMAP_USABLE) {
			uint64_t start_page = entry->base / PAGE_SIZE;
			uint64_t page_count = entry->length / PAGE_SIZE;

			for (uint64_t j = 0; j < page_count; j++) {
				bitmap_unset(start_page + j);
			}
		}	
	}

	uint64_t bitmap_start_page = (uint64_t)bitmap_phys_addr / PAGE_SIZE;
	uint64_t bitmap_page_count = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;

	for (uint64_t i = 0; i < bitmap_page_count; i++) {
		bitmap_set(bitmap_start_page + i);
	}

	for (uint64_t i = 0; i < 256; i++) {
		bitmap_set(i);
	}

	terminal.printf("PMM Initialized. Bitmap at Phys: %x\n", bitmap_phys_addr);
}

void* PMM::alloc_page() {
	for (uint64_t i = 0; i < bitmap_size; i++) {
		if (bitmap[i] == 0xFF) {
			continue;
		}

		for (uint64_t bit = 0; bit < 8; bit++) {
			if (!(bitmap[i] >> bit & 1)) {
				uint64_t page_number = (i * 8) + bit;
				bitmap_set(page_number);
				uint64_t phys_addr = page_number * PAGE_SIZE;
				return (void*)phys_addr;
			}
		}
	}

	terminal.printf("CRITICAL: Out of Memory!\n");
	return NULL;
}

void PMM::free_page(void* address) {
	uint64_t phys_addr = (uint64_t)address;
	uint64_t page_number = phys_addr / PAGE_SIZE;
	bitmap_unset(page_number);
}

