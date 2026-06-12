#pragma once
#include "cpu/interrupts.h"

#define MAX_PROCESSES 8

enum ProcessState {
	PROC_UNUSED = 0,
	PROC_READY
};

struct Process {
	registers context;
	uint64_t cr3;
	ProcessState state;
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
};

extern Scheduler scheduler;
