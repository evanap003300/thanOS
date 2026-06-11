#include <stdint.h>

volatile uint64_t counter;

extern "C" void _start() {
	// Spin for a bit: while this runs in ring 3 the kernel's timer
	// interrupt keeps firing, so the cursor keeps blinking
	for (counter = 0; counter < 300000000ull; counter++) { }

	// hlt is privileged. In ring 3 this must die with a #GP -
	// the crash screen is the proof we were unprivileged.
	asm volatile ("hlt");

	for (;;) { }
}
