# thanOS — Architecture Traces

This is a study doc, not a reference doc. The rule for every trace:

1. Write the trace **from memory** — numbered steps, from start event to end state.
2. Mark anything you're unsure about with `(?)`.
3. Verify against the code (or have Claude diff it) and correct it in place.
4. Anything you got wrong goes in the Misconception Log at the bottom — that's the flashcard seed.

Before starting a new trace, re-tell one old trace in ~2 minutes (out loud or on
paper). A trace counts as "done" when it survives a from-memory retelling a week
after you wrote it.

---

## Trace 1 — A keypress becomes a glyph

**Start:** you press `a`. **End:** the glyph is on screen and the cursor has advanced.

**Territory:** `src/cpu/idt_asm.asm`, `src/cpu/idt.cpp`, `src/cpu/interrupts.cpp`,
`src/drivers/pic.cpp`, `src/drivers/keyboard.cpp`, `src/shell/shell.cpp`,
`src/graphics/render.cpp`

Questions you should be able to answer afterward:

- How does the CPU know to stop what it's doing, and how does it find the handler?
  Why does the keyboard land on vector 33 and not vector 1?
- What does the CPU push on the stack by itself, and what does your assembly stub
  push on top? Why the dummy error code?
- Why must the handler send an EOI, and what happens if it never does?
- Why is it OK that the handler never saves the SSE/FPU registers?
- What turns a scancode into a character, and what turns a character into pixels?

### The trace (corrected — session 2026-06-10)

First attempt: skeleton right (device → PIC → save registers → handle → glyph →
framebuffer), four gaps — thought the PIC knew *which* key; said "rewrote" the
PIC instead of *remapped*; missing the IDT dispatch entirely; missing EOI/iretq.
Retell target:

1. The keyboard's microcontroller sends scancode `0x1E` ('a', scancode set 1) to
   the PS/2 controller, which latches it at port `0x60` and raises the IRQ1 line
   into the master PIC.
2. The PIC checks: line 1 unmasked, nothing higher-priority in service → raises
   INTR to the CPU. The PIC only ever knows the *line*, never the key.
3. The CPU is asleep in `kmain`'s `hlt` loop with IF=1 → wakes, acknowledges
   (INTA); the PIC answers with vector 33 (remapped base `0x20` + line 1).
4. The CPU indexes the IDT with 33: the gate holds `isr33`'s address + selector
   `0x08` (kernel code in the GDT) + flags `0x8E`. The CPU itself pushes SS,
   RSP, RFLAGS, CS, RIP and clears IF (interrupt gate), then jumps to the stub.
5. `isr33`: `push 0` (dummy error code), `push 33`, `jmp isr_common_stub`
   (`idt_asm.asm:114`).
6. Common stub pushes the 15 GPRs, then `mov rdi, rsp` — the stack *is* the
   `registers` struct — and calls `isr_handler`.
7. `isr_handler`: `int_num == 33` → `keyboard_handler_main()`
   (`interrupts.cpp:47`).
8. `inb(0x60)` reads the scancode. High bit set would mean key-*release* → EOI
   and return (`keyboard.cpp:46`).
9. `keymap[0x1E]` → `'a'`; erase the cursor; `shell.on_key_pressed('a')`.
10. Shell: not `\n`/`\b` → append to the 256-byte command buffer and echo via
    `terminal.draw_char` (`shell.cpp:119`).
11. `draw_char`: `font8x16['a']` is 16 bytes, one per row, MSB = leftmost pixel;
    each bit writes color or background straight into framebuffer memory at
    `(cursor_y+i)*width + (cursor_x+j)`. Limine handed over this pointer at boot
    and is never called again. `cursor_x += 8`. (`render.cpp:26`)
12. Back in the keyboard handler: redraw cursor, reset `timer_ticks`, send EOI
    to the PIC (`keyboard.cpp:62`) — skip this and IRQ1 never fires again.
13. Stub unwinds: pop 15 GPRs, `add rsp, 16` (discard int# + error code),
    `iretq` pops RIP/CS/RFLAGS/RSP/SS → IF=1 again.
14. The CPU lands back in the `hlt` loop and sleeps until the next interrupt.

---

## Trace 2 — `new` becomes bytes in a DRAM chip

**Start:** kernel code runs `String s = "hi";`. **End:** the characters live in a
physical page frame.

**Territory:** `src/std/string.cpp`, `src/memory/heap.cpp`, `src/cpu/vmm.cpp`,
`src/cpu/pmm.cpp`

Questions you should be able to answer afterward:

- What happens, in order, between `operator new` and `malloc` returning a pointer?
  How does the free-list find a block, and when does it split one?
- Who decided the heap lives at virtual `0x100000000`, and what had to happen at
  boot for that address to be usable at all?
- Walk a virtual address through PML4 → PDPT → PD → PT. What lives in a page
  table entry besides the physical address?
- What is the HHDM, and why can't the VMM just dereference a physical address
  it reads out of a page table entry?
- Where does a fresh physical page come from? How does the PMM know it's free?
- What is the TLB, and why does `map_memory` end with `invlpg`?

### The trace (write from memory)

*(yours)*

---

## Trace 3 — `cat` pulls bytes off a spinning (virtual) disk

**Start:** you type `cat` and hit Enter. **End:** the contents of `TEST.TXT` are on
screen.

**Territory:** `src/shell/shell.cpp`, `src/fs/fat32.cpp`, `src/fs/mbr.cpp`,
`src/drivers/ata.cpp`, `src/utils/io.cpp`

Questions you should be able to answer afterward:

- What do `inb`/`outb` actually do — what bus do they talk on? How is PIO
  different from DMA?
- Describe the register dance to read sector N: which ports, in what order, and
  what you poll for (BSY/DRQ) before touching the data port.
- What's in sector 0, and how do you find where the FAT32 partition starts?
- From the BPB, how do you compute where cluster N lives on disk? Why does
  cluster numbering start at 2?
- What is the FAT itself (the actual table), and why can the current code only
  read single-cluster files?

### The trace (write from memory)

*(yours)*

---

## Trace 4 — Power button to blinking prompt (capstone)

**Start:** QEMU powers on. **End:** `root@thanOS:/>` with a blinking cursor.

**Territory:** `Makefile`, `linker.ld`, `limine.conf`, `src/kernel.cpp`, and every
init call it makes

Questions you should be able to answer afterward:

- What does Limine do for you that a legacy bootloader path would make you do by
  hand? What state is the CPU already in when `kmain` runs?
- How do the request/response structs reach the bootloader? Why do they need
  their own linker section?
- Why does the kernel live at `0xffffffff80000000`, and which compiler flags
  exist because of that choice?
- Recite the init order in `kmain`. For each step: what breaks if you move it
  one step earlier? (Could the heap come before the VMM? The IDT after `sti`?)
- After `shell.init()` returns, the kernel just executes `hlt` in a loop. How
  does anything ever happen again?

### The trace (write from memory)

*(yours)*

---

## Trace 5 — `run` executes a program from disk(ish)

**Start:** you type `run` and hit Enter. **End:** `Program returned: 1342`.

**Territory:** `Makefile`, `user/test.cpp`, `user/user.ld`, `src/shell/shell.cpp`,
`src/fs/vfs.cpp`, `src/loader/elf.cpp`, `src/cpu/pmm.cpp`, `src/cpu/vmm.cpp`

Questions you should be able to answer afterward:

- How did `test.elf` get into the initrd, and how does the kernel find its bytes
  at runtime? (Build pipeline → tar → Limine module → `Vector<File>`.)
- The loader does four things per PT_LOAD segment — name them, in order, and
  say why each exists.
- Why is the answer 1342? Which variable proves `.data` loading, which proves
  BSS zeroing, and why are they `volatile`?
- Why is the binary linked at `0x140000000` instead of the classic `0x400000`?
  What compiler flag follows from that address choice?
- Why must the file be ET_EXEC, and what does GCC produce by default instead?
- What privilege level did the program run at — and inside what *context* did
  it execute? Why is that second answer surprising?

### The trace (write from memory)

*(yours — good opener for next session)*

---

## Trace 6 — `mt` runs two programs at once

**Start:** you type `mt` and hit Enter. **End:** `ABBAABBB...` interleaving, two
clean exits, shell prompt back.

**Territory:** `src/proc/process.{h,cpp}`, `src/cpu/interrupts.cpp` (timer branch),
`src/cpu/syscall.cpp` (exit), `src/shell/shell.cpp`, `user/proc_a.cpp`

Questions you should be able to answer afterward:

- What *is* a process in thanOS, concretely — which struct, which fields?
- A context switch happens inside which interrupt handler, and what single line
  of code actually performs it?
- Why do the letters appear in clumps of ~3 instead of strict ABAB?
- How does a brand-new process start running when nothing ever "calls" it?
  (What five values were forged, and what instruction consumes them?)
- Why is process 0 special, and when does it run?
- What does `exit` do now that it didn't do before — and what *still* doesn't
  happen on exit? (Hint: who frees the stack pages?)
- Why is it (currently) safe for all processes to share one TSS rsp0 stack —
  and what kernel feature would break that?
- (post-isolation) proc_a and proc_b are both linked at 0x140000000 and both
  mutate `msg` at the same virtual address. Walk the page-table reason this
  doesn't collide. Why did the kernel heap have to move to the higher half
  first? What one instruction did the context switch gain, and why is it the
  expensive one?

### The trace (write from memory)

*(yours)*

---

## Misconception Log

What you got wrong is the most valuable thing this doc captures. One row each;
these become flashcards.

| Date | What I thought | What's actually true | Where |
|------|----------------|----------------------|-------|
| 2026-06-10 | The PIC knows which key was pressed | The PIC only knows which IRQ *line* fired; the handler reads the scancode itself from port 0x60 | `keyboard.cpp:46` |
| 2026-06-10 | We "rewrote the PIC to understand the keyboard" | We *remapped* its vector base (IRQs 0–15 → vectors 32–47) so IRQs don't collide with CPU exception vectors, then unmasked lines 0 and 1 | `pic.cpp:47` |
| 2026-06-10 | (missing) how the CPU finds the handler | Vector 33 indexes the IDT; the gate holds the stub address + selector 0x08; the CPU pushes SS/RSP/RFLAGS/CS/RIP by itself | `idt.cpp:42` |
| 2026-06-10 | (missing) EOI | The handler must send EOI or the PIC keeps that line "in service" and never delivers it again — one keypress, then dead | `pic.cpp:81` |
| 2026-06-10 | "printf wraps Limine's framebuffer" | Limine hands over a raw pixel-buffer pointer at boot and is never called again; the echo path is `draw_char` (printf is for command output), rendering the font bitmap directly into memory | `render.cpp:26` |
| 2026-06-10 | (didn't know) what a "vector" is | Just an index 0–255 into the IDT. CPU exceptions own 0–31 by architecture; the PIC delivers `base + line` (32 + 1 = 33 = keyboard); `int N` delivers N | `pic.cpp:59`, `idt.cpp:93` |
| 2026-06-10 | (didn't know) what IF is | The Interrupt Flag bit in RFLAGS — the CPU's master "may I be interrupted?" switch. `sti` sets it, `cli` clears it; interrupt gates auto-clear it on entry; `iretq` restores it. `sti`+`hlt` = sleep until an event (kmain's loop); `cli`+`hlt` = sleep forever (`hcf`) | `kernel.cpp:80`, `kernel.cpp:35` |
| 2026-06-10 | "rdi points to where the process is executing" | rdi is the SysV ABI *first-argument* register; the stub does `mov rdi, rsp` after the last push, so it points at saved r15 — the lowest address of the `registers` struct. "Where it was executing" is RIP, which is saved *inside* that struct | `idt_asm.asm:49` |
| 2026-06-10 | Debugging: reached for logging before using the symptom | The blinking cursor already proves the shared machinery works (IDT loaded, PIC remapped, sti, common stub, dispatch). Suspects are only where the keyboard path *diverges* from the timer path: `pic_unmask(1)`, gate 33, EOI(1), keymap | `kernel.cpp:77`, `idt.cpp:93` |
| 2026-06-10 | "Without .init_array, globals are left uninitialized" | BSS globals are guaranteed *zero* (Limine zeroes it) — never garbage. The real failure is zeroed-but-**unconstructed**: constructors whose work amounts to more than writing zeros (e.g. a global `String` that must allocate) silently never happen | `kernel.cpp:52` |
| 2026-06-10 | "A bit per page tells the OS whether we're in userland" | Two separate things conflated. *Which ring we're in* is a property of the **CPU right now** — the low 2 bits of CS (that's the 0x23 on the crash screen). The *page* U-bit says whether ring 3 may **touch that page**. One is "who am I," the other is "what may I reach" | `gdt_asm.asm` (push 0x23), `vmm.h:7` |
| 2026-06-10 | "...tells the **OS**" | The OS checks nothing at runtime — it isn't even running while the user program runs. The **CPU hardware** checks CPL against the tables on every instruction and memory access. The kernel only *writes the rulebook* (GDT, page tables, IDT) and *handles the aftermath* (exceptions) | `cpu/gdt.cpp`, `cpu/vmm.cpp` |
| 2026-06-11 | "The exception handler runs on the user's stack" | The kernel never trusts a user stack (unmapped? hostile?). On any ring 3→0 entry the CPU **first** loads RSP from `TSS.rsp0` — the 16 KiB `interrupt_stack` — found via the task register (`ltr 0x28`). That stack switch is the TSS's entire job today | `gdt.cpp` |
| 2026-06-11 | (didn't know) which IDT entry catches a privilege violation | Vector **13** = #GP, fixed by the architecture (0–31 are all reserved for exceptions). The crash screen's "Interrupt Number: 13" was saying so | `interrupts.cpp` |
