#pragma once
#include "cpu/interrupts.h"
#include "cpu/spinlock.h"
#include "cpu/gdt.h"

#define MAX_PROCESSES 8

enum ProcessState {
	PROC_UNUSED = 0,
	PROC_READY,     // runnable, waiting for a core
	PROC_RUNNING,   // on a core right now
	PROC_BLOCKED,   // sleeping in read(), woken by a keypress
	PROC_WAITING    // sleeping in wait(), woken by a child's exit
};

struct Process {
	registers context;
	uint64_t cr3;
	ProcessState state;

	// Which core this process is on while it is PROC_RUNNING
	int core;

	// While blocked in read(): the user virtual address (in this
	// process's address space) where the next key should land
	uint64_t read_buf;

	// Slot of the process that spawned this one, 0 for the kernel
	// shell, or -1 once the parent is gone
	int parent;

	// Children that exited before this process called wait(), and the
	// exit code of the latest one
	int exited_children;
	int exit_code;
};

class Scheduler {
public:
	// One run queue shared by every core. Slot 0 is not a real
	// process: it stands for the kernel (the parent of anything the
	// shell starts) and holds the kernel's CR3.
	Process processes[MAX_PROCESSES];

	// What each core is running: a slot number, or 0 when it is in
	// its idle loop
	int current[MAX_CORES];

	// Where each core's idle loop was when it was last switched away
	// from. Core 0's idle loop is kmain, which also drains the
	// keyboard ring; the others only hlt. The frame points into that
	// core's own stack, so it is never resumed anywhere else.
	registers idle_context[MAX_CORES];

	// Guards everything above. Every core takes it on every tick.
	Spinlock sched_lock;

	void init();
	int create(void* entry, uint64_t cr3);

	// Slot running on the calling core (0 = idle / the kernel)
	int current_pid();

	// tick() is the locked entry from the timer ISR; schedule()
	// is the worker and ASSUMES sched_lock is already held.
	void tick(registers* regs);
	void schedule(registers* regs);

	void exit_current(registers* regs, int code);
	void block_current_for_read(registers* regs, uint64_t user_buf);
	bool deliver_key(char c);
	void wait_current(registers* regs);

	// The shell's `ps`: every live process, its state, and its core
	void print_processes();
};

extern Scheduler scheduler;
