#pragma once
#include "cpu/interrupts.h"

// Syscall numbers match Linux x86_64 on purpose
#define SYS_READ  0
#define SYS_WRITE 1
#define SYS_EXIT  60
#define SYS_WAIT  61

// Custom: Linux splits process creation into fork + execve;
// thanOS does it in one call (for now)
#define SYS_SPAWN 200

void syscall_handler(registers* regs);
