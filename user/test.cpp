#include "syscalls.h"

extern "C" void _start() {
	write("Hello from userland!\n");
	write("(this text traveled through int 0x80)\n");
	exit(42);
}
