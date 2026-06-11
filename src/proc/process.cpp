#include "proc/process.h"
#include "cpu/pmm.h"
#include "cpu/vmm.h"

Scheduler scheduler;

void Scheduler::init() {
	// The kernel idle loop is process 0. Its context gets saved
	// the first time we switch away from it.
	processes[0].state = PROC_READY;
	current = 0;
}

int Scheduler::create(void* entry) {
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

	// Each slot gets its own 16 KiB user stack. Stacks are reused
	// across a slot's lifetimes and never freed (no cleanup yet).
	uint64_t stack_base = 0x150000000 + (uint64_t)slot * 0x10000;
	uint64_t stack_top = stack_base + 0x4000;

	for (uint64_t page = stack_base; page < stack_top; page += 0x1000) {
		if (kernel_vmm.virt_to_phys((void*)page) == NULL) {
			void* phys = pmm.alloc_page();
			if (phys == NULL) {
				return -1;
			}
			kernel_vmm.map_memory((void*)page, phys, PTE_USER_SUPER);
		}
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

	current = next;

	// The context switch: what iretq restores is now someone else
	*regs = processes[current].context;
}

void Scheduler::exit_current(registers* regs) {
	processes[current].state = PROC_UNUSED;
	schedule(regs);
}
