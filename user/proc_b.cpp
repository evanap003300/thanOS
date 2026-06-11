#include "syscalls.h"

extern "C" void _start() {
	for (int i = 0; i < 30; i++) {
		for (volatile uint64_t d = 0; d < 5000000; d++) { }
		write("B");
	}
	exit(0);
}
