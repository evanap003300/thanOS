#include "shell/shell.h"
#include "graphics/render.h"
#include "std/string.h"
#include "fs/vfs.h"
#include "drivers/ata.h"
#include "fs/mbr.h"

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
	} else if (String(buffer) == "cyan") {
		File* config = VFS::open("./theme.cfg");
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
			       

	} else if (String(buffer) == "dump") {
		uint8_t sector_data[512];
		ATA::read_sectors(0, 1, sector_data);

		terminal.printf("Disk Data (LBA): ");
		for (int i = 0; i < 20; i++) {
			terminal.printf("%x ", sector_data[i]);
		}
		terminal.printf("\n");
	} else if (String(buffer) == "mount") {
		uint32_t partition_lba = MBRParser::find_first_partition();
		if (partition_lba > 0) {
			terminal.printf("FAT32 Partition found at Sector: %d\n", partition_lba);
		} else {
			terminal.printf("Error: No FAT32 partition found.\n");
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

