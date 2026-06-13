#pragma once
#include "cpu/interrupts.h"

#define MAX_PROCESSES 8

enum ProcessState {
	PROC_UNUSED = 0,
	PROC_READY,
	PROC_BLOCKED
};

struct Process {
	registers context;
	uint64_t cr3;
	ProcessState state;

	// While blocked in read(): the user virtual address (in this
	// process's address space) where the next key should land
	uint64_t read_buf;
};

class Scheduler {
public:
	// Slot 0 is the kernel idle loop (kmain's hlt loop)
	Process processes[MAX_PROCESSES];
	int current;

	void init();
	int create(void* entry, uint64_t cr3);
	void schedule(registers* regs);
	void exit_current(registers* regs);
	void block_current_for_read(registers* regs, uint64_t user_buf);
	bool deliver_key(char c);
};

extern Scheduler scheduler;
