#include "fs/tar.h"
#include "graphics/render.h"

size_t octal_to_int(const char* str) {
	size_t size = 0;
	while (*str) { 
		int byte = *str - '0';
		if (byte >= 0 && byte < 8) {
			size = size * 8 + byte;
		}
		str++;
	}

	return size;
}

namespace Tar {
	void parse(uint64_t address) {
		terminal.printf("[Tar Parser Started]\n");

		while (true) {
			TarHeader* header = (TarHeader*)address;

			if (header->name[0] == '\0') {
				break;
			}

			size_t size = octal_to_int(header->size);

			char* content = (char*)(address + 512);
		
			if (header->typeflag == '5') {
				terminal.printf("DIR: %s\n", header->name);
				address += 512;
				continue;
			}

			if (size > 0 && header->typeflag == '0') {
				terminal.printf("Content:\n");
				terminal.printf("File: %s | Size: %d bytes\n", header->name, size);
				for (size_t i = 0; i < size && i < 30; i++) {
					terminal.printf("%c", content[i]);
				}
				terminal.printf("\n");
			}

			size_t size_in_blocks = (size + 511) / 512;

			address += 512 + (size_in_blocks * 512);
		}

		terminal.printf("[Tar Parser Finished]\n");
	}
}
