#include <stdint.h>

// Lives in .data: the value 1000 is stored in the ELF file itself
volatile int data_value = 1000;

// Lives in .bss: not stored in the file, the loader must zero it
volatile int bss_value;

extern "C" int _start() {
	return data_value + bss_value + 342;
}
