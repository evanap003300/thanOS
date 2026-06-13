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
* wait() syscall: a parent process blocks until its child exits (completes the spawn/wait process model)

## Current to-dos:

### Robustness (finish the process model) — DONE
* User-fault isolation: a faulting process is killed, the kernel survives (`crash` command demos it)
* Process reaping: a dead process's pages, tables, and PML4 are freed on exit (`mem` command shows zero leak)

### Foundation for apps
- [ ] Growable heap
- [ ] Faster, readable timer (PIT ~1000 Hz)
- [ ] Keyboard event queue

### Projects
- [ ] A simple graphical game (snake/pong)
- [ ] FAT32 write support (persistence, note app)
- [ ] Doom

### Big tracks (later)
- [ ] Multicore (SMP): boot the other cores via Limine, per-core TSS/scheduler, + spinlocks around shared kernel data (PMM, heap, scheduler, terminal)
- [ ] Network stack (e1000 -> ARP/ICMP -> UDP/DNS -> TCP -> HTTP)
- [ ] FAT32: follow cluster chains (multi-cluster files)
- [ ] Doom :)

