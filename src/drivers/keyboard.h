#pragma once

// Drain the key ring: each queued key goes to a process blocked in
// read(), or to the kernel shell if nobody is waiting. Called from
// kmain's loop only - it is the ring's single consumer.
void keyboard_pump();

// True if the IRQ handler has queued keys that keyboard_pump() has not
// consumed yet.
bool keyboard_pending();
