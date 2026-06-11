#include <stdint.h>

// rax = number, rdi/rsi/rdx = args, return lands back in rax
static inline int64_t syscall3(int64_t number, uint64_t a1, uint64_t a2, uint64_t a3) {
	int64_t ret;
	asm volatile ("int $0x80"
		: "=a"(ret)
		: "a"(number), "D"(a1), "S"(a2), "d"(a3)
		: "memory");
	return ret;
}

static uint64_t strlen(const char* s) {
	uint64_t n = 0;
	while (s[n]) n++;
	return n;
}

static int64_t write(const char* s) {
	return syscall3(1, 1, (uint64_t)s, strlen(s));
}

static void exit(int code) {
	syscall3(60, (uint64_t)code, 0, 0);
}

extern "C" void _start() {
	write("Hello from userland!\n");
	write("(this text traveled through int 0x80)\n");
	exit(42);

	for (;;) { }
}
