#include "render.h"
#define FONT8x16_IMPLEMENTATION
#include "font.h"

void draw_char(struct limine_framebuffer* framebuffer, char c, int x, int y, uint32_t color) {
	if (!framebuffer) return;

	uint32_t* fb_ptr = (uint32_t*)framebuffer->address;
	uint64_t stride = framebuffer->width;

	int HEIGHT = 16;
	int WIDTH = 8;

	for (uint16_t i = 0; i < HEIGHT; i++) {
		unsigned char font_row = font8x16[(int)c][i];
		for (uint16_t j = 0; j < WIDTH; j++) {
			if ((font_row >> (7-j)) & 1) {
				uint64_t offset = (y+i) * stride + (x+j);
				fb_ptr[offset] = color;
			}	
		}
	}
}
