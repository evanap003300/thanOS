#include "interrupts.h"
#include "render.h"

extern "C" void isr_handler(registers* regs) {
	terminal.clear();
	terminal.printf("--- CPI EXCEPTION RECEIVED ---\n");

	terminal.printf("Interrupt Number: %d\n", regs->interrupt_number);
	terminal.printf("Error Code:       %x\n", regs->error_code);
	
	terminal.printf("RIP (Instruction): %x\n", regs->rip);

	int int_num = regs->interrupt_number;

	if (int_num == 0) {
		terminal.printf("Description: Divide by Zero\n");
	} else if (int_num == 13) {
		terminal.printf("Description: General Protectoin Fault\n");
	} else if (int_num == 14) {
		terminal.printf("Description: Page Fault\n");
	}

	terminal.printf("--------------------------\n");
	terminal.printf("System Halted.\n");

	while(true) { 
		asm ("hlt");
	}
}
