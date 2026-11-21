#include "proc/process.h"
#include "cpu/pmm.h"
#include "cpu/vmm.h"
#include "cpu/smp.h"
#include "graphics/render.h"
#include "drivers/keyboard.h"

Scheduler scheduler;

void Scheduler::init() {
	// Slot 0 is the kernel. It is never scheduled; it only marks the
	// slot as taken and remembers the kernel's address space.
	processes[0].state = PROC_READY;
	__asm__ volatile ("mov %%cr3, %0" : "=r"(processes[0].cr3));

	// Every core starts out idle
	for (int i = 0; i < MAX_CORES; i++) {
		current[i] = 0;
	}
}

int Scheduler::current_pid() {
	return current[smp_core_id()];
}

int Scheduler::create(void* entry, uint64_t cr3) {
	uint64_t f = sched_lock.acquire_irq();

	int slot = -1;
	for (int i = 1; i < MAX_PROCESSES; i++) {
		if (processes[i].state == PROC_UNUSED) {
			slot = i;
			break;
		}
	}

	if (slot == -1) {
		sched_lock.release_irq(f);
		return -1;
	}

	// Every process sees its 16 KiB stack at the same virtual
	// address - separate address spaces make that legal.
	PageTableManager user_vmm((PageTable*)cr3);
	uint64_t stack_base = 0x150000000;
	uint64_t stack_top = stack_base + 0x4000;

	for (uint64_t page = stack_base; page < stack_top; page += 0x1000) {
		void* phys = pmm.alloc_page();
		if (phys == NULL) {
			sched_lock.release_irq(f);
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
	p->core = -1;
	// Whoever is running on this core is the new process's parent:
	// 0 when the kernel shell started it, a user spawner otherwise
	p->parent = current[smp_core_id()];
	p->exited_children = 0;
	p->exit_code = 0;

	// From this line on, any core's next tick may pick it up
	p->state = PROC_READY;

	sched_lock.release_irq(f);
	return slot;
}

void Scheduler::tick(registers* regs) {
	// The locked entry point from the timer ISR.
	uint64_t f = sched_lock.acquire_irq();
	schedule(regs);
	sched_lock.release_irq(f);
}

void Scheduler::schedule(registers* regs) {
	int core = smp_core_id();
	int cur = current[core];

	// Park whoever was running here. A process that just blocked or
	// exited was already dealt with by the caller, so only a process
	// that is still RUNNING goes back in the queue. The moment it is
	// READY another core may take it - that is safe because its whole
	// state is this saved frame; nothing of it lives on our stack.
	if (cur == 0) {
		idle_context[core] = *regs;
	} else if (processes[cur].state == PROC_RUNNING) {
		processes[cur].context = *regs;
		processes[cur].state = PROC_READY;
	}

	// Round robin over the shared queue, starting after the slot we
	// just ran; idle (0) if nothing is ready. The exception is core 0
	// with keys waiting: its idle loop is the key ring's only
	// consumer, so it has to run before anyone can be handed them.
	int next = 0;
	if (!(core == 0 && keyboard_pending())) {
		for (int off = 1; off <= MAX_PROCESSES; off++) {
			int i = (cur + off) % MAX_PROCESSES;
			if (i != 0 && processes[i].state == PROC_READY) {
				next = i;
				break;
			}
		}
	}

	if (next != 0) {
		// Claim it before the lock is dropped so no other core can
		processes[next].state = PROC_RUNNING;
		processes[next].core = core;
	}

	if (next == cur) {
		// Same process (or still idle): the frame iretq is about
		// to restore is already the right one
		return;
	}

	terminal.printf("[cpu%d %d>%d]", core, cur, next);

	current[core] = next;

	// The context switch: what iretq restores is now someone else
	uint64_t next_cr3;
	if (next == 0) {
		*regs = idle_context[core];
		next_cr3 = processes[0].cr3;
	} else {
		*regs = processes[next].context;
		next_cr3 = processes[next].cr3;
	}

	// Writing CR3 flushes the TLB, so skip it when the next
	// process lives in the same address space
	uint64_t active_cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r"(active_cr3));
	if (active_cr3 != next_cr3) {
		__asm__ volatile ("mov %0, %%cr3" : : "r"(next_cr3) : "memory");
	}
}

void Scheduler::exit_current(registers* regs, int code) {
	uint64_t f = sched_lock.acquire_irq();

	int cur = current[smp_core_id()];
	int parent = processes[cur].parent;
	uint64_t dead_cr3 = processes[cur].cr3;

	processes[cur].state = PROC_UNUSED;

	// This slot will be reused. Any children still alive must not
	// report their exit to whoever lands in it next.
	for (int i = 1; i < MAX_PROCESSES; i++) {
		if (processes[i].state != PROC_UNUSED && processes[i].parent == cur) {
			processes[i].parent = -1;
		}
	}

	if (parent > 0 && processes[parent].state != PROC_UNUSED) {
		if (processes[parent].state == PROC_WAITING) {
			// The parent is asleep in wait(): this is the event
			// that wakes it - hand over the exit code through
			// its parked frame
			processes[parent].context.rax = (uint64_t)code;
			processes[parent].state = PROC_READY;
		} else {
			// The parent has not called wait() yet. With the
			// child free to run on another core that is a real
			// race, so remember the exit instead of losing it.
			processes[parent].exited_children++;
			processes[parent].exit_code = code;
		}
	}

	// We're still standing on the dying process's page tables. Step
	// onto the kernel's address space (the shared kernel half keeps
	// our code + stack mapped) before tearing the dead one down.
	__asm__ volatile ("mov %0, %%cr3" : : "r"(processes[0].cr3) : "memory");
	free_address_space(dead_cr3);

	schedule(regs);
	sched_lock.release_irq(f);
}

void Scheduler::wait_current(registers* regs) {
	uint64_t f = sched_lock.acquire_irq();

	int cur = current[smp_core_id()];

	// A child already finished (it may have run to completion on
	// another core before we got here): return its code right away
	if (processes[cur].exited_children > 0) {
		processes[cur].exited_children--;
		regs->rax = (uint64_t)processes[cur].exit_code;
		sched_lock.release_irq(f);
		return;
	}

	// No children at all: nobody would ever wake us
	bool has_child = false;
	for (int i = 1; i < MAX_PROCESSES; i++) {
		if (processes[i].state != PROC_UNUSED && processes[i].parent == cur) {
			has_child = true;
			break;
		}
	}

	if (!has_child) {
		regs->rax = (uint64_t)-1;
		sched_lock.release_irq(f);
		return;
	}

	// Same shape as block_current_for_read: park the frame (its RIP
	// already points past the syscall), sleep, let someone else run.
	// The waking happens in exit_current when a child dies.
	processes[cur].context = *regs;
	processes[cur].state = PROC_WAITING;

	schedule(regs);
	sched_lock.release_irq(f);
}

void Scheduler::block_current_for_read(registers* regs, uint64_t user_buf) {
	uint64_t f = sched_lock.acquire_irq();

	int cur = current[smp_core_id()];

	// Park the frame as-is: its RIP already points at the
	// instruction after int 0x80, so waking = resuming
	processes[cur].context = *regs;
	processes[cur].state = PROC_BLOCKED;
	processes[cur].read_buf = user_buf;

	schedule(regs);
	sched_lock.release_irq(f);
}

bool Scheduler::deliver_key(char c) {
	uint64_t f = sched_lock.acquire_irq();

	for (int i = 1; i < MAX_PROCESSES; i++) {
		if (processes[i].state != PROC_BLOCKED) {
			continue;
		}

		// The buffer address only means something in THIS process's
		// address space, and its CR3 is not active here - so
		// translate through its page tables and write via the HHDM
		PageTableManager proc_vmm((PageTable*)processes[i].cr3);
		void* phys = proc_vmm.virt_to_phys((void*)processes[i].read_buf);

		if (phys == NULL) {
			sched_lock.release_irq(f);
			return false;
		}

		*(char*)((uint64_t)phys + get_hhdm_offset()) = c;

		// read() returns 1: written into the parked frame's rax,
		// which becomes the syscall's return value on wake.
		// Whichever core ticks next picks the process up.
		processes[i].context.rax = 1;
		processes[i].state = PROC_READY;

		sched_lock.release_irq(f);
		return true;
	}

	sched_lock.release_irq(f);
	return false;
}

void Scheduler::print_processes() {
	// Copy what we need under the lock, print after letting go of it:
	// drawing text is slow and every core's tick wants this lock
	ProcessState states[MAX_PROCESSES];
	int cores[MAX_PROCESSES];

	uint64_t f = sched_lock.acquire_irq();
	for (int i = 0; i < MAX_PROCESSES; i++) {
		states[i] = processes[i].state;
		cores[i] = processes[i].core;
	}
	sched_lock.release_irq(f);

	terminal.printf("%d cores online\n", smp_core_count());
	terminal.printf("PID  STATE    CORE\n");

	for (int i = 1; i < MAX_PROCESSES; i++) {
		if (states[i] == PROC_UNUSED) {
			continue;
		}

		if (states[i] == PROC_RUNNING) {
			terminal.printf("%d    running  %d\n", i, cores[i]);
		} else if (states[i] == PROC_READY) {
			terminal.printf("%d    ready    -\n", i);
		} else if (states[i] == PROC_BLOCKED) {
			terminal.printf("%d    blocked  -   (in read)\n", i);
		} else {
			terminal.printf("%d    waiting  -   (in wait)\n", i);
		}
	}
}
