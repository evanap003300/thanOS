#include "cpu/interrupts.h"
#include "graphics/render.h"
#include "drivers/pic.h"
#include "utils/io.h"

extern "C" void keyboard_handler_main();

uint64_t timer_ticks = 0;

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
		timer_ticks++;

		if (timer_ticks % 9 == 0) {
			terminal.toggle_cursor();
		}

		pic_send_eoi(0);
		return;
	} else if (int_num == 33) {
		keyboard_handler_main();
		return;
	} else {
		terminal.printf("Unknown Interrupt: %d\n", int_num);
		if (int_num >= 32 && int_num <= 47) {
			pic_send_eoi(int_num - 32); 
		}
	}
	
}
