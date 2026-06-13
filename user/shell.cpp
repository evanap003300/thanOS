#include "syscalls.h"

static bool streq(const char* a, const char* b) {
	int i = 0;
	while (a[i] && b[i]) {
		if (a[i] != b[i]) {
			return false;
		}
		i++;
	}
	return a[i] == b[i];
}

extern "C" void _start() {
	char buffer[64];
	char key[2] = { 0, 0 };

	write("\nthanOS userland shell. commands: hello, mt, exit\n");

	for (;;) {
		write("ush> ");

		int len = 0;

		// Read a line, echoing each key as it arrives
		for (;;) {
			read(&key[0]);

			char c = key[0];

			if (c == '\n') {
				write("\n");
				break;
			}

			if (len < 63) {
				buffer[len] = c;
				len++;
				write(key);
			}
		}

		buffer[len] = 0;

		if (streq(buffer, "hello")) {
			spawn("./test.elf");
			wait();
			write("[ush] child finished\n");
		} else if (streq(buffer, "mt")) {
			spawn("./proc_a.elf");
			spawn("./proc_b.elf");
		} else if (streq(buffer, "exit")) {
			write("bye from ush!\n");
			exit(0);
		} else if (len > 0) {
			write("unknown command: ");
			write(buffer);
			write("\n");
		}
	}
}
