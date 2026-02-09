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

extern Vector<File> file_system;
