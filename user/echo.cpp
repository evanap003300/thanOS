#include "syscalls.h"

static char buf[2] = { 0, 0 };

extern "C" void _start() {
	write("echo: type keys, q to quit\n");

	for (;;) {
		read(&buf[0]);

		if (buf[0] == 'q') {
			break;
		}

		write("[");
		write(buf);
		write("]");
	}

	write("\nbye!\n");
	exit(0);
}
