#pragma once 
#include <stdint.h>
#include "limine.h"
#include <stdarg.h>

class Renderer {
	public:
		struct limine_framebuffer* framebuffer;
		uint32_t color;
		uint32_t cursor_x;
		uint32_t cursor_y;

		Renderer(struct limine_framebuffer* target_framebuffer);

		void put_pixel(uint32_t x, uint32_t y, uint32_t color);
		void draw_char(char c);
		void clear();
		void next_line();
		void print(const char* str);
		void scroll();

		void print_number(int number);
		void print_hex(uint32_t number);

		void printf(const char* format, ...);
};
