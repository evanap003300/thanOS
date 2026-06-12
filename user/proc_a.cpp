#include "syscalls.h"

// Same virtual address in proc_b - separate address spaces
// are the only reason this works
static char msg[3] = { 'A', '0', 0 };

extern "C" void _start() {
	for (int i = 0; i < 20; i++) {
		for (volatile uint64_t d = 0; d < 5000000; d++) { }
		msg[1] = (char)('0' + (i % 10));
		write(msg);
	}
	exit(0);
}
