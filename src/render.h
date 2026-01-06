#pragma once 
#include <stdint.h>
#include "limine.h"

void draw_char(struct limine_framebuffer* framebuffer, char c, int x, int y, uint32_t color);
