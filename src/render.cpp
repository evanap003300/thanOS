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

	for (uint16_t i = 0; i < HEIGHT; i++) {
		unsigned char font_row = font8x16[(unsigned char)c][i];
		for (uint16_t j = 0; j < WIDTH; j++) {
			if ((font_row >> (7-j)) & 1) {
				uint64_t offset = (cursor_y+i) * stride + (cursor_x+j);
				fb_ptr[offset] = color;
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
        	fb_ptr[i] = 0x0E0E0E;
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
	// shift all the text on the screen up and remove the text on the top
	
	cursor_x = 0;
	cursor_y -= 16;
}
