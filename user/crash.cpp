#include "syscalls.h"

extern "C" void _start() {
	write("crash: about to dereference a null pointer...\n");

	volatile int* bad = (volatile int*)0;
	*bad = 42;   // page fault from ring 3 - should kill only this process

	write("crash: this line should never run\n");
	exit(0);
}
