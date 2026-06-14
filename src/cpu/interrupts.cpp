#include "cpu/interrupts.h"
#include "cpu/syscall.h"
#include "graphics/render.h"
#include "drivers/pic.h"
#include "utils/io.h"
#include "proc/process.h"
#include "cpu/apic.h"

extern "C" void keyboard_handler_main();

uint64_t timer_ticks = 0;

extern "C" void isr_handler(registers* regs) {
	uint64_t int_num = regs->interrupt_number;
	
	if (int_num < 32) {
		// Fault from ring 3 (CS low bits == 3): a user process crashed.
		// Kill just that process and reschedule - the kernel survives.
		if (regs->cs & 3) {
			terminal.printf("[process %d faulted: exception %d] killed\n", scheduler.current, (int)int_num);
			scheduler.exit_current(regs, -1);
			return;
		}

		// Fault from ring 0: a kernel bug, genuinely fatal.
		terminal.clear();
		terminal.printf("--- KERNEL PANIC ---\n");

		terminal.printf("Interrupt Number: %d\n", regs->interrupt_number);
		terminal.printf("Error Code:       %x\n", regs->error_code);
	
		terminal.printf("RIP (Instruction): %x\n", regs->rip);
		terminal.printf("CS (Ring):         %x\n", regs->cs);

		int int_num = regs->interrupt_number;

		if (int_num == 0) {
			terminal.printf("Description: Divide by Zero\n");
		} else if (int_num == 13) {
			terminal.printf("Description: General Protection Fault\n");
		} else if (int_num == 14) {
			terminal.printf("Description: Page Fault\n");
		}

		terminal.printf("--------------------------\n");
		terminal.printf("System Halted.\n");

		while (true) {
			asm ("hlt");
		}
	} else if (int_num == 32) {
		timer_ticks++;

		if (timer_ticks % 9 == 0) {
			terminal.toggle_cursor();
		}

		pic_send_eoi(0);

		// Every tick is a scheduling opportunity: this may swap
		// the frame that iretq is about to restore (tick() takes
		// the scheduler lock around the switch)
		scheduler.tick(regs);
		return;
	} else if (int_num == 33) {
		keyboard_handler_main();
		return;
	} else if (int_num == 48) {
		// LAPIC timer: the scheduling tick (replaces the legacy PIT).
		// ~100 Hz, so toggle the cursor every 50 ticks (~0.5s).
		timer_ticks++;
		if (timer_ticks % 50 == 0) {
			terminal.toggle_cursor();
		}

		scheduler.tick(regs);
		lapic_eoi();
		return;
	} else if (int_num == 128) {
		// Software interrupt: no PIC involved, no EOI needed
		syscall_handler(regs);
		return;
	} else {
		terminal.printf("Unknown Interrupt: %d\n", int_num);
		if (int_num >= 32 && int_num <= 47) {
			pic_send_eoi(int_num - 32); 
		}
	}
	
}
