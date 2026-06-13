# thanOS - a modern 64-bit operating system.
## Languages & Technology
* C++ 
* Limine (64-bit bootloader)
* Some assembly

## Current status: 
* Working shell
* Custom interrupts (user can now input to screen, handle division by zero, etc)
* Physical and virtual memory managers (multi-layer pageing)
* Malloc, new, and delete implementations for memory management
* Custom string and vector implementation
* Read from disk in Fat-32 format
* ELF loader (loads programs from the initrd)
* Userland: ring 3 execution with GDT user segments + TSS
* Syscalls via int 0x80 (write, exit — Linux-compatible numbers)
* Preemptive multitasking: timer-driven round-robin scheduler (`mt` runs two processes at once)
* Per-process address spaces: every process gets its own page tables (CR3 switch in the scheduler), programs all link at the same virtual address

* Blocking read() syscall: processes sleep waiting for keyboard input (interactive `echo` program)
* spawn() syscall: a process can launch another process
* Userland shell (`ush`): a shell running in ring 3, reads input and spawns child programs entirely via syscalls

## Current to-dos:
- [ ] wait() syscall (parent blocks until child exits)
- [ ] Doom prerequisites: growable heap, faster timer, keyboard queue
- [ ] FAT32: follow cluster chains (multi-cluster files)
- [ ] Doom :)

