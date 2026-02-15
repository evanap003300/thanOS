#pragma once 
#include "std/string.h"
#include "std/vector.h"
#include <stdint.h>

struct File {
	String name;
	size_t size;
	uint8_t* data;
	bool is_directory;
};

class VFS {
public: 
	static File* open(const char* filename);
};

extern Vector<File> file_system;
