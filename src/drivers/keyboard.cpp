#include "drivers/keyboard.h"
#include "utils/io.h"
#include "utils/spsc_ring.h"
#include "drivers/pic.h"
#include "graphics/render.h"
#include "shell/shell.h"
#include "proc/process.h"
#include "cpu/apic.h"
#include "cpu/spinlock.h"

// The Translation Table: Scancode -> ASCII
// This maps the first 58 keys (0-9, A-Z, some symbols)
const char keymap[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', /* Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0, /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, /* Right shift */
  '*',
    0, /* Alt */
  ' ', /* Space bar */
    0, /* Caps lock */
    0, /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0, /* < ... F10 */
    0, /* 69 - Num lock*/
    0, /* Scroll Lock */
    0, /* Home key */
    0, /* Up Arrow */
    0, /* Page Up */
  '-',
    0, /* Left Arrow */
    0,
    0, /* Right Arrow */
  '+',
    0, /* 79 - End key*/
    0, /* Down Arrow */
    0, /* Page Down */
    0, /* Insert Key */
    0, /* Delete Key */
    0,   0,   0,
    0, /* F11 Key */
    0, /* F12 Key */
    0, /* All other keys are undefined */
};

extern uint64_t timer_ticks;

// Keys travel from the interrupt handler to the kernel's main loop
// through this ring: one producer (the IRQ handler below), one consumer
// (keyboard_pump), and no lock between them.
static SpscRing<char, 64> key_ring;

// Producer. This runs in interrupt context, so it does the minimum:
// read the scancode, translate it, queue it, acknowledge the interrupt.
// What the key means is decided later, outside the handler.
extern "C" void keyboard_handler_main() {
	uint8_t scancode = inb(0x60);

	// High bit set = key release; only presses are queued
	if (!(scancode & 0x80) && keymap[scancode] != 0) {
		// A full ring drops the key - better than waiting for
		// space inside an interrupt handler
		key_ring.push(keymap[scancode]);
	}

	lapic_eoi();
}

bool keyboard_pending() {
	return !key_ring.empty();
}

// Consumer. Runs in kmain's loop (process 0), never in an interrupt
// handler. The keyboard IRQ can fire at any point in here and push
// another key; the ring is built for exactly that.
void keyboard_pump() {
	char c;

	while (key_ring.pop(c)) {
		// Routing a key can run a whole shell command or wake a
		// process. That code was written to run with interrupts
		// off (it used to live in the IRQ handler), so keep that
		// promise for as long as it runs
		uint64_t flags = Spinlock::save_and_disable_interrupts();

		terminal.draw_cursor(false);

		// A process blocked in read() has first claim on the key;
		// the shell only gets it if nobody is waiting
		if (!scheduler.deliver_key(c)) {
			shell.on_key_pressed(c);
		}

		terminal.draw_cursor(true);
		timer_ticks = 0;

		Spinlock::restore_interrupts(flags);
	}
}
