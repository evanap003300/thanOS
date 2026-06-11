#pragma once
#include "cpu/interrupts.h"

// Syscall numbers match Linux x86_64 on purpose
#define SYS_WRITE 1
#define SYS_EXIT  60

void syscall_handler(registers* regs);
