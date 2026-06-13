#include "cpu/syscall.h"
#include "graphics/render.h"
#include "proc/process.h"
#include "fs/vfs.h"
#include "cpu/vmm.h"
#include "loader/elf.h"

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
	scheduler.exit_current(regs, (int)regs->rdi);
}

// wait() -> blocks until a child process exits; returns its exit code
static void sys_wait(registers* regs) {
	scheduler.wait_current(regs);
}

// read(fd, buf, count) -> rdi, rsi, rdx. Blocks until one key
// arrives; fd and count are ignored (always delivers 1 byte).
static void sys_read(registers* regs) {
	uint64_t buf = regs->rsi;

	if (buf < 0x140000000 || buf >= 0x180000000) {
		regs->rax = (uint64_t)-1;
		return;
	}

	scheduler.block_current_for_read(regs, buf);
}

// spawn(path) -> rdi: launch another program as a new process.
// Returns the new pid, or -1.
static void sys_spawn(registers* regs) {
	uint64_t user_path = regs->rdi;

	if (user_path < 0x140000000 || user_path >= 0x180000000) {
		regs->rax = (uint64_t)-1;
		return;
	}

	// The caller's address space is active during its own syscall,
	// so its string is directly readable - just bound the copy
	char path[64];
	int len = 0;
	const char* src = (const char*)user_path;

	while (len < 63 && src[len]) {
		path[len] = src[len];
		len++;
	}
	path[len] = 0;

	File* program = VFS::open(path);
	if (program == nullptr) {
		regs->rax = (uint64_t)-1;
		return;
	}

	uint64_t cr3 = create_address_space();
	if (cr3 == 0) {
		regs->rax = (uint64_t)-1;
		return;
	}

	void* entry = ELF::load(program->data, cr3);
	if (entry == nullptr) {
		regs->rax = (uint64_t)-1;
		return;
	}

	regs->rax = (uint64_t)scheduler.create(entry, cr3);
}

void syscall_handler(registers* regs) {
	switch (regs->rax) {
		case SYS_SPAWN:
			sys_spawn(regs);
			break;
		case SYS_READ:
			sys_read(regs);
			break;
		case SYS_WRITE:
			sys_write(regs);
			break;
		case SYS_EXIT:
			sys_exit(regs);
			break;
		case SYS_WAIT:
			sys_wait(regs);
			break;
		default:
			terminal.printf("Unknown syscall: %d\n", (int)regs->rax);
			regs->rax = (uint64_t)-1;
	}
}
