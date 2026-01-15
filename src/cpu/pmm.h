#pragma once
#include <stdint.h>
#include <stddef.h>
#include "limine.h"

#define PAGE_SIZE 4096

class PMM {
public:
	uint8_t* bitmap;
	uint64_t bitmap_size;
	uint64_t total_ram_pages;

	void init();
	void* alloc_page();
	void free_page(void* address);

private:
	void bitmap_set(uint64_t bit);
	void bitmap_unset(uint64_t bit);
	bool bitmap_test(uint64_t bit);
};

extern PMM pmm; 
