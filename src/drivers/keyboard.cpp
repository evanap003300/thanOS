#include "utils/io.h"
#include "drivers/pic.h"
#include "graphics/render.h"
#include "shell/shell.h"

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

extern "C" void keyboard_handler_main() {
	uint8_t scancode = inb(0x60);

	if (scancode & 0x80) {
		pic_send_eoi(1);
		return;
	}

	terminal.draw_cursor(false);

	if (scancode < 128 && keymap[scancode] != 0) {
		shell.on_key_pressed(keymap[scancode]);
	}

	terminal.draw_cursor(true);
	timer_ticks = 0;

	pic_send_eoi(1);
}
