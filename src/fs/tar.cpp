#include "fs/tar.h"
#include "graphics/render.h"
#include "fs/vfs.h"

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
		terminal.printf("FS: [Tar Parser Started]\n");

		while (true) {
			TarHeader* header = (TarHeader*)address;

			if (header->name[0] == '\0') {
				break;
			}

			size_t size = octal_to_int(header->size);

			char* content = (char*)(address + 512);
		
			if (header->typeflag == '5') {
				terminal.printf("TAR: DIR: %s\n", header->name);
				address += 512;
				continue;
			}

			if (size > 0 && header->typeflag == '0') {
				terminal.printf("TAR: File: %s | Size: %d bytes\n", header->name, size);

				// A file is only "text" if its bytes happen to be printable
				bool is_text = true;
				for (size_t i = 0; i < size && i < 30; i++) {
					unsigned char c = (unsigned char)content[i];
					if ((c < 0x20 || c > 0x7E) && c != '\n' && c != '\t' && c != '\r') {
						is_text = false;
						break;
					}
				}

				terminal.printf("TAR: %s: ", header->name);
				if (is_text) {
					for (size_t i = 0; i < size && i < 30; i++) {
						if (content[i] != '\n') {
							terminal.printf("%c", content[i]);
						}
					}
					terminal.printf("\n");
				} else {
					terminal.printf("(binary)\n");
				}
			}

			// Add file to file system
                        File f;
                        f.name = header->name;
                        f.size = size;
                        f.data = (uint8_t*)content;
			f.is_directory = (header->typeflag == '5');

			if (!f.is_directory) {
				file_system.push_back(f);
				terminal.printf("[FS] Loaded file: %s\n", f.name.c_str());
			}

			size_t size_in_blocks = (size + 511) / 512;

			address += 512 + (size_in_blocks * 512);
		}

		terminal.printf("FS: [Tar Parser Finished]\n");
	}
}
