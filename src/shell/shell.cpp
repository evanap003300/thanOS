#include "shell/shell.h"
#include "graphics/render.h"
#include "std/string.h"

Shell shell;

void Shell::init() {
	buffer_index = 0;
	for (int i = 0; i < 256; i++) {
		buffer[i] = 0;
	}
	terminal.printf("\nroot@thanOS:/>");
}

void Shell::handle_backspace() {
	if (buffer_index > 0) {
		buffer_index--;
		buffer[buffer_index] = 0;
		terminal.backspace();
	}
}


void Shell::execute() {
	if (String(buffer) == "whoami") {
		terminal.printf("ap_ev\n");
	} else if (String(buffer) == "clear") {
		terminal.clear();
	} else if (String(buffer) == "load_theme") {
		terminal.printf("loaded theme");	
	} else {
		terminal.printf("Unknown command.\n");
	}
}

void Shell::on_key_pressed(char c) {
	if (buffer_index >= 255) {
		return;
	}
	
	if (c == '\n') {
		terminal.next_line();
		execute();
		buffer_index = 0;

		for (int i = 0; i < 256; i++) {
			buffer[i] = 0;
		}
		
		terminal.printf("root@thanOS:/>");
	} else if (c == '\b') {
		handle_backspace();
	} else {
		buffer[buffer_index] = c;
		buffer_index++;
		buffer[buffer_index] = 0;
		terminal.draw_char(c);
	}
}


