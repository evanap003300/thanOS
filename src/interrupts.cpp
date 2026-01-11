#include "interrupts.h"
#include "render.h"
#include "pic.h"
#include "io.h"

extern "C" void isr_handler(registers* regs) {
	uint64_t int_num = regs->interrupt_number;
	
	if (int_num < 32) {
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
	} else if (int_num == 32) { 
		pic_send_eoi(0);
		return;
	} else if (int_num == 33) {
		uint8_t scancode = inb(0x60);
		terminal.printf("Key: %x ", scancode);
		pic_send_eoi(1);
		return;
	} else {
		terminal.printf("Unknown Interrupt: %d\n", int_num);
		if (int_num >= 32 && int_num <= 47) {
			pic_send_eoi(int_num - 32); 
		}
	}
	
}
