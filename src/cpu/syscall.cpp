#include "cpu/syscall.h"
#include "graphics/render.h"
#include "proc/process.h"

// write(fd, buf, count) -> rdi, rsi, rdx. fd is ignored for now.
static void sys_write(registers* regs) {
	uint64_t buf = regs->rsi;
	uint64_t count = regs->rdx;

	// Never trust a user pointer: only accept addresses inside the
	// user program / user stack region, or userland could ask the
	// kernel to read its own memory on its behalf
	if (buf < 0x140000000 || buf + count > 0x180000000) {
		regs->rax = (uint64_t)-1;
		return;
	}

	const char* str = (const char*)buf;
	for (uint64_t i = 0; i < count; i++) {
		terminal.printf("%c", str[i]);
	}

	regs->rax = count;
}

// exit(code) -> rdi. Retire the slot, schedule someone else into
// the frame iretq is about to restore.
static void sys_exit(registers* regs) {
	terminal.printf("\nProcess %d exited with code: %d\n", scheduler.current, (int)regs->rdi);

	scheduler.exit_current(regs);
}

void syscall_handler(registers* regs) {
	switch (regs->rax) {
		case SYS_WRITE:
			sys_write(regs);
			break;
		case SYS_EXIT:
			sys_exit(regs);
			break;
		default:
			terminal.printf("Unknown syscall: %d\n", (int)regs->rax);
			regs->rax = (uint64_t)-1;
	}
}
