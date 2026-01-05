#include <stdint.h>
#include <stddef.h>
#include <limine.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0
};

static void hcf(void) {
	for(;;) {
		asm ("hlt");
	}
}

extern "C" void kmain(void) {
	if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
		hcf();
	}

	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

	uint32_t* pixels = (uint32_t*)framebuffer->address;
	
	for (int i = 0; i < 100; i++) {
		pixels[i] = 0xFF6a0DAD;	
	}

	hcf();

}
