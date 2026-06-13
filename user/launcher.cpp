#include "syscalls.h"

extern "C" void _start() {
	write("launcher: spawning test.elf...\n");

	int64_t pid = spawn("./test.elf");

	if (pid < 0) {
		write("launcher: spawn failed\n");
	} else {
		write("launcher: spawned!\n");
	}

	exit(0);
}
