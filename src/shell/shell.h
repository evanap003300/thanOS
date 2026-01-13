#pragma once 
#include <stdint.h>

class Shell {
public:
	char buffer[256];
	int buffer_index;
	void init();
	void on_key_pressed(char c);
	void execute();
	void handle_backspace();
};

extern Shell shell;
