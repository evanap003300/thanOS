#include "proc/process.h"
#include "cpu/pmm.h"
#include "cpu/vmm.h"
#include "graphics/render.h"

Scheduler scheduler;

void Scheduler::init() {
	// The kernel idle loop is process 0. Its context gets saved
	// the first time we switch away from it.
	processes[0].state = PROC_READY;
	__asm__ volatile ("mov %%cr3, %0" : "=r"(processes[0].cr3));
	current = 0;
}

int Scheduler::create(void* entry, uint64_t cr3) {
	int slot = -1;
	for (int i = 1; i < MAX_PROCESSES; i++) {
		if (processes[i].state == PROC_UNUSED) {
			slot = i;
			break;
		}
	}

	if (slot == -1) {
		return -1;
	}

	// Every process sees its 16 KiB stack at the same virtual
	// address - separate address spaces make that legal.
	// Physical pages are never freed (no cleanup yet).
	PageTableManager user_vmm((PageTable*)cr3);
	uint64_t stack_base = 0x150000000;
	uint64_t stack_top = stack_base + 0x4000;

	for (uint64_t page = stack_base; page < stack_top; page += 0x1000) {
		void* phys = pmm.alloc_page();
		if (phys == NULL) {
			return -1;
		}
		user_vmm.map_memory((void*)page, phys, PTE_USER_SUPER);
	}

	// Hand-forge the frame iretq will pop the first time this
	// process is scheduled: same five values jump_to_user pushed
	Process* p = &processes[slot];
	p->context = {};
	p->context.rip = (uint64_t)entry;
	p->context.cs = 0x23;
	p->context.rflags = 0x202;
	p->context.rsp = stack_top;
	p->context.ss = 0x1B;

	p->cr3 = cr3;
	// Whoever is running right now is this process's parent:
	// the kernel shell runs as process 0, a user spawner as itself
	p->parent = current;
	p->state = PROC_READY;

	return slot;
}

void Scheduler::schedule(registers* regs) {
	// Park the state of whoever was running
	if (processes[current].state == PROC_READY) {
		processes[current].context = *regs;
	}

	// Round robin over user slots; idle (0) only if nothing is ready
	int next = 0;
	for (int off = 1; off <= MAX_PROCESSES; off++) {
		int i = (current + off) % MAX_PROCESSES;
		if (i != 0 && processes[i].state == PROC_READY) {
			next = i;
			break;
		}
	}

	if (next == current) {
		return;
	}

	terminal.printf("[%d>%d]", current, next);

	current = next;

	// The context switch: what iretq restores is now someone else
	*regs = processes[current].context;

	// Writing CR3 flushes the TLB, so skip it when the next
	// process lives in the same address space
	uint64_t active_cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r"(active_cr3));
	if (active_cr3 != processes[current].cr3) {
		__asm__ volatile ("mov %0, %%cr3" : : "r"(processes[current].cr3) : "memory");
	}
}

void Scheduler::exit_current(registers* regs, int code) {
	int parent = processes[current].parent;
	uint64_t dead_cr3 = processes[current].cr3;

	processes[current].state = PROC_UNUSED;

	// If our parent is asleep in wait(), this is the event that
	// wakes it - hand it our exit code through its parked frame
	if (parent >= 0 && processes[parent].state == PROC_WAITING) {
		processes[parent].context.rax = (uint64_t)code;
		processes[parent].state = PROC_READY;
	}

	// We're still standing on the dying process's page tables. Step
	// onto the kernel's address space (process 0's, shared kernel half
	// keeps our code + stack mapped) before tearing the dead one down.
	__asm__ volatile ("mov %0, %%cr3" : : "r"(processes[0].cr3) : "memory");
	free_address_space(dead_cr3);

	schedule(regs);
}

void Scheduler::wait_current(registers* regs) {
	// Same shape as block_current_for_read: park the frame (its RIP
	// already points past the syscall), sleep, let someone else run.
	// The waking happens in exit_current when a child dies.
	processes[current].context = *regs;
	processes[current].state = PROC_WAITING;

	schedule(regs);
}

void Scheduler::block_current_for_read(registers* regs, uint64_t user_buf) {
	// Park the frame as-is: its RIP already points at the
	// instruction after int 0x80, so waking = resuming
	processes[current].context = *regs;
	processes[current].state = PROC_BLOCKED;
	processes[current].read_buf = user_buf;

	schedule(regs);
}

bool Scheduler::deliver_key(char c) {
	for (int i = 1; i < MAX_PROCESSES; i++) {
		if (processes[i].state != PROC_BLOCKED) {
			continue;
		}

		// The buffer address only means something in THIS process's
		// address space, and its CR3 may not be active right now -
		// so translate through its page tables and write via the HHDM
		PageTableManager proc_vmm((PageTable*)processes[i].cr3);
		void* phys = proc_vmm.virt_to_phys((void*)processes[i].read_buf);

		if (phys == NULL) {
			return false;
		}

		*(char*)((uint64_t)phys + get_hhdm_offset()) = c;

		// read() returns 1: written into the parked frame's rax,
		// which becomes the syscall's return value on wake
		processes[i].context.rax = 1;
		processes[i].state = PROC_READY;

		return true;
	}

	return false;
}
