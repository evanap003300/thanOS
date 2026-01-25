#pragma once
#include <stdint.h>
#include <stddef.h> 

struct MemorySegmentHeader {
	uint64_t length;
	struct MemorySegmentHeader* next;
	struct MemorySegmentHeader* last;
	bool free;	

	void CombineForward();
	void CombineBackward();

	MemorySegmentHeader* Split(size_t splitLength);
};

void InitializeHeap(void* heapAddress, size_t pageCount);
void* malloc(size_t size);
void free(void* address);

void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* p);
void operator delete[](void* p);
void operator delete(void* p, size_t);
void operator delete[](void* p, size_t);
