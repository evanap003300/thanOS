#pragma once
#include <stdint.h>

// rax = number, rdi/rsi/rdx = args, return comes back in rax.
// Numbers match the Linux x86_64 syscall ABI.
static inline int64_t syscall3(int64_t number, uint64_t a1, uint64_t a2, uint64_t a3) {
	int64_t ret;
	asm volatile ("int $0x80"
		: "=a"(ret)
		: "a"(number), "D"(a1), "S"(a2), "d"(a3)
		: "memory");
	return ret;
}

static inline uint64_t strlen(const char* s) {
	uint64_t n = 0;
	while (s[n]) n++;
	return n;
}

static inline int64_t write(const char* s) {
	return syscall3(1, 1, (uint64_t)s, strlen(s));
}

static inline void exit(int code) {
	syscall3(60, (uint64_t)code, 0, 0);
	for (;;) { }
}
