#include "shell.h"
#include "render.h"

Shell shell;

bool str_eq(const char* str1, const char* str2) {
	int i = 0;
	while (str1[i] != 0 && str2[i] != 0) {
		if (str1[i] != str2[i]) {
			return false;
		}
		i++;
	}
	return str1[i] == str2[i];
}

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
        if (str_eq(buffer, "whoami")) {
                terminal.printf("evanap\n");
        } else if (str_eq(buffer, "clear")) {
                terminal.clear();
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


