#include "render.h"
#define FONT8x16_IMPLEMENTATION
#include "font.h"

Renderer::Renderer(struct limine_framebuffer* framebuffer_) {
	framebuffer = framebuffer_;
	color = 0xFFFFFFFF;
	cursor_x = 0;
	cursor_y = 0;	
}


void Renderer::put_pixel(uint32_t x, uint32_t y, uint32_t color) {
	if (x >= framebuffer->width || y >= framebuffer->height) return;
	
	uint32_t* fb_ptr = (uint32_t*)framebuffer->address;
	fb_ptr[x + (y * framebuffer->width)] = color;	
}

void Renderer::draw_char(char c) {
	if (!framebuffer) return;

	uint32_t* fb_ptr = (uint32_t*)framebuffer->address;
	uint64_t stride = framebuffer->width;

	uint16_t HEIGHT = 16;
	uint16_t WIDTH = 8;

	uint32_t bg_color = 0x0E0E0E0E;

	for (uint16_t i = 0; i < HEIGHT; i++) {
		unsigned char font_row = font8x16[(unsigned char)c][i];
		for (uint16_t j = 0; j < WIDTH; j++) {
			uint64_t offset = (cursor_y+i) * stride + (cursor_x+j);
			if ((font_row >> (7-j)) & 1) {
				fb_ptr[offset] = color;
			} else {
				fb_ptr[offset] = bg_color;
			}	
		}
	}

	cursor_x += 8;

	if (cursor_x >= framebuffer->width) {
		next_line();
	}
}

void Renderer::clear() {
	uint32_t* fb_ptr = (uint32_t*)framebuffer->address;
    	uint64_t width = framebuffer->width;
    	uint64_t height = framebuffer->height;

    	for (uint64_t i = 0; i < width * height; i++) {
        	fb_ptr[i] = 0xFF0E0E0E;
    	}

	cursor_x = 0;
	cursor_y = 0;
}

void Renderer::next_line() {
	cursor_x = 0;
	cursor_y += 16;

	if (cursor_y >= framebuffer->height) {
		scroll();
	}
}

void Renderer::print(const char* str) {
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\n') {
			next_line();
		} else {
			draw_char(str[i]);
		}
	}
}

void Renderer::scroll() {
	uint32_t* fb_ptr = (uint32_t*)framebuffer->address; 
	uint32_t bg_color = 0xFF0E0E0E;
	uint32_t color = bg_color;

	for (uint64_t i = 16; i < framebuffer->height; i++) {
		for (uint64_t j = 0; j < framebuffer->width; j++) {
			uint64_t position = i*framebuffer->width + j;
			color = fb_ptr[position];
			fb_ptr[position - (16*framebuffer->width)] = color;
		}
	}

	uint64_t last_row_start = (framebuffer->height - 16) * framebuffer->width;

	for (uint64_t i = last_row_start; i < last_row_start + (16 * framebuffer->width); i++) {
		fb_ptr[i] = bg_color;
	}

	cursor_x = 0;
	cursor_y -= 16;
}

void Renderer::print_number(int number) {
	if (number == 0) {
		draw_char('0');
		return;
	}

	if (number < 0) {
		draw_char('-');
		number = -1 * number;
	}

	char buffer[16];
	int i = 0;

	while (number > 0) {
		buffer[i] = (number % 10) + '0';
		number /= 10;
		i++;
	}
	
	i--;

	while (i >= 0) {
		draw_char(buffer[i]);
		i--;
	}
	
}


