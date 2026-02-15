#include "fs/vfs.h"

extern Vector<File> file_system; 

File* VFS::open(const char* filename) {
	for (size_t i = 0; i < file_system.size(); i++) {
		File* f = &file_system[i];

		if (f->name == filename) {
			return f;
		}
	}

	return nullptr;
}
