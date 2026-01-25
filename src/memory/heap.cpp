#include "memory/heap.h"
#include "cpu/pmm.h"
#include "cpu/vmm.h"
#include "graphics/render.h"

void* heapStart;
void* heapEnd;
MemorySegmentHeader* firstHeader;

void InitializeHeap(void* heapAddress, size_t pageCount) {
	void* pos = heapAddress;

	for (size_t i = 0; i < pageCount; i++) {
		void* phys = pmm.alloc_page();
		kernel_vmm.map_memory(pos, phys);
		pos = (void*)((size_t)pos + 0x1000);
	}
	
	MemorySegmentHeader* firstSegment = (MemorySegmentHeader*)heapAddress;

	size_t total_size = pageCount * 0x1000;

	firstSegment->length = totalSize - sizeof(MemorySegmentHeader);
	firstSegment->next = NULL;
	firstSegment->last = NULL;
	firstSegment->free = true;

	firstHeader = firstSegment;
	heapStart = heapAddress;
}

void* malloc(size_t size) {
	if (size % 8 != 0) {
		size -= size % 8;
		size += 8;
	}

	if (size == 0) {
		
		return NULL;
	}

	MemorySegmentHeader* current = firstHeader;

	while (true) {
		if (current->free) {
			if (current->length > size) {
				current->Split(size);
				current->free = false;
				return (void*)((uint64_t)current + sizeof(MemorySegmentHeader));	
			}

			if (current->length == size) {
				current->free = false;
				return (void*)((uint64_t)current + sizeof(MemorySegmentHeader));
			}
		}

		if (current->next == NULL) {
			break;
		}

		current = current->next;
	}

	return NULL;
}

MemorySegmentHeader* MemorySegmentHeader::Split(size_t splitLength) {
	if (splitLength < 0x10) {
		return NULL;
	}

	int64_t splitSegmentLength = length - splitLength - sizeof(MemorySegmentHeader);

	if (splitSegmentLength < 0) {
		return NULL;
	}

	MemorySegmentHeader* newSplitHeader = (MemorySegmentHeader*) ((size_t)this + sizeof(MemorySegmentHeader) + splitLength);

	newSplitHeader->length = splitSegmentLength;
	newSplitHeader->free = true;
	newSplitHeader->next = this->next;
	newSplitHeader->last = this;

	if (this->next != NULL) {
		this->next->last = newSplitHeader;
	}

	this->next = newSplitHeader;
	this->lenght = splitLength;

	return newSplitHeader;
}

void* operator new(size_t size) {
	return malloc(size);
}

void* operator new[](size_t size) {
	return malloc(size);
}

void operator delete(void* p) {
	free(p);
}

void operator delete[](void* p) {
	free(p);
}
