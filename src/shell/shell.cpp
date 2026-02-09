#include "shell/shell.h"
#include "graphics/render.h"
#include "std/string.h"
#include "fs/vfs.h"

Shell shell;

extern Vector<File> file_system;

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
	} else if (String(buffer) == "load") {
		File* config = nullptr;

		for (size_t i = 0; i < file_system.size(); i++) {
			if (file_system[i].name == "./theme.cfg") {
		       		config = &file_system[i];
				break;
			}
		}

		if (config == nullptr) {
			terminal.printf("Error: theme.cfg not found.\n");
		} else {
			char* content = (char*)config->data;

			if (strncmp(content, "CYAN", 4) == 0) {
				terminal.setColor(0x00FFFF);
				terminal.printf("Theme set to CYAN.\n");	
			} else if (strncmp(content, "GREEN", 5) == 0) {
				terminal.setColor(0xFF0000);
				terminal.printf("Theme set to RED.\n");
			} else {
				terminal.printf("Unknown theme: %s\n", content);
			}
		}
			       

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

