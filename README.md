# thanOS

A 64-bit x86 kernel written from scratch in freestanding C++ with a little
assembly. No libc, no STL. It boots every core through Limine, runs ring-3
programs in their own address spaces, and schedules them preemptively.

![thanOS booting on 4 cores](docs/boot.png)

## What works

**Memory**
- Bitmap physical page allocator.
- 4-level paging. Each process gets its own PML4; the kernel half is shared and
  CR3 is switched in the scheduler, so every program links at the same virtual
  address.
- Kernel heap: a free list that splits and coalesces blocks, behind `malloc` /
  `free` and `operator new` / `delete`.

**Processes**
- ELF64 loader, and ring 3 through user segments in the GDT plus a TSS per core.
- Syscalls over `int 0x80` with Linux's numbers: `read`, `write`, `exit`,
  `wait`, and a `spawn` that stands in for fork + exec.
- Preemptive round-robin scheduler driven by the LAPIC timer. `read()` and
  `wait()` block, and the process is woken by a keypress or a child exiting.
- Fault isolation: a process that faults is killed and the kernel keeps
  running. On exit its pages, page tables and PML4 are freed.
- `ush`, a shell that runs in ring 3 and reads input and launches children
  using nothing but syscalls.

**Interrupts and SMP**
- GDT, IDT, exception handlers, PS/2 keyboard.
- All cores are started through Limine's MP protocol, each with its own TSS,
  kernel stack and local APIC.
- The legacy PIC is retired: the LAPIC timer is calibrated against the PIT, and
  the keyboard is routed through the IOAPIC, which is located by parsing the
  ACPI MADT.
- Spinlocks (plain, interrupt-safe, and a re-entrant one for the terminal)
  guard the page allocator, the heap and the scheduler.
- The extra cores do not run processes yet. After boot they join a lock
  contention test (the last lines of the screenshot above) and park. Scheduling
  across cores is the next step.

**Storage**
- An initrd: a tar archive loaded as a Limine module and exposed through a
  small VFS. User programs are loaded from it.
- ATA PIO driver, MBR parsing and read-only FAT32 (single-cluster files for
  now).

**The rest**
- Framebuffer text renderer with an 8x16 bitmap font, and a `printf`.
- `String` and `Vector`, since there is no standard library to lean on.

## A session

![shell session](docs/session.png)

- `mount`, `ls`, `cat` read the FAT32 partition on the attached disk.
- `crash` starts a program that dereferences a null pointer. The page fault
  kills that process only, and `mem` reports the same free page count before
  and after.
- `ush` starts the userland shell. `hello` there spawns a child and waits for
  it. The child's output reaches the screen through `int 0x80`.
- The `[1>0]` markers are the scheduler tracing its context switches.

## Build and run

You need an `x86_64-elf` cross toolchain (g++ and ld), `nasm`, `xorriso`, QEMU,
and `parted`, `dosfstools` and `mtools` for the FAT32 test disk.

```sh
make -C Limine   # once, builds the limine host tool
make run         # builds the kernel + user programs, makes the ISO and disk, boots QEMU with 4 cores
```

## Layout

```
src/cpu/       GDT, IDT, interrupts, physical and virtual memory, syscalls, APIC, IOAPIC, SMP, locks
src/proc/      processes and the scheduler
src/memory/    kernel heap
src/loader/    ELF loader
src/fs/        tar initrd, VFS, MBR, FAT32
src/drivers/   keyboard, ATA, legacy PIC
src/graphics/  framebuffer text renderer
src/std/       String, Vector
src/shell/     the in-kernel shell
user/          ring-3 programs and the syscall wrappers they use
```

## Next

- [ ] Run processes on every core (per-core run state, parallel execution)
- [ ] Lock-free SPSC ring for keyboard input
- [ ] Growable heap
- [ ] FAT32 cluster chains and write support
- [ ] Network stack (e1000, then ARP/ICMP, UDP, TCP)
- [ ] Doom
