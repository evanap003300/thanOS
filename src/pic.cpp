#include "pic.h"
#include "io.h"

void pic_mask(uint8_t irq_line) {
	uint16_t port;
	uint8_t  value;

	if (irq_line < 8) {
		port = PIC1_DATA;	
	} else {
		port = PIC2_DATA;
		irq_line -= 8;
	}

	// Get the current mask
	value = inb(port);
	
	// Mask the bit on the irq_line
	value |= (1 << irq_line);

	// Write back the new mask
	outb(port, value);
}


void pic_unmask(uint8_t irq_line) {
	uint16_t port;
        uint8_t  value;

        if (irq_line < 8) {
                port = PIC1_DATA; 
        } else {
                port = PIC2_DATA;
                irq_line -= 8;
        }

        // Get the current mask
        value = inb(port);

        // Unmask the bit on the irq_line
        value &= ~(1 << irq_line);

        // Write back the new mask
        outb(port, value);
}

void pic_remap() {
	// Save current data state
	uint8_t a1 = inb(PIC1_DATA);
	uint8_t a2 = inb(PIC2_DATA);

	// Tell PICs to listen for 3 more ICWs 
	outb(PIC1_COMMAND, ICW1_INIT);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT);
	io_wait();

	// Remap the PIC to new ports
	outb(PIC1_DATA, 0x20);
	io_wait();
	outb(PIC2_DATA, 0x28);
	io_wait();

	// Confiure Cacading at IRQ 2
	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();

	// Restore masks
	outb(PIC1_DATA, a1);
	outb(PIC2_DATA, a2);
}

void pic_send_eoi(uint8_t irq) {
	if (irq >= 8) {
		outb(PIC2_COMMAND, PIC_EOI);
	}
	
	outb(PIC1_COMMAND, PIC_EOI);
}

