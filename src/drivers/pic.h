#pragma once
#include <stdint.h> 

// Master PIC 
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21

// Slave PIC (0b10 IRQ of Master PIC) 
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1 


// ICWs (Initialization Command Words):

// ICW1: 0b00010001, (0x11)
// Bit 4: Init bit.
// Bit 0 = We will send ICW4 later
#define ICW1_INIT    0x11

// ICW4: 0x01 => 8086 mode
// Running on x86 PC, not MCS-85 controller
#define ICW4_INIT    0x01

// End of Interrupt (EOI)
// Set to the command port after handling an interrupt
#define PIC_EOI      0x20 

// Function header defs:

// Remap the PIC interrupts to avoid CPU Conflicts
void pic_remap();

// Signals to the PIC to reopen the IRQ port after finishing an interrupt
void pic_send_eoi(uint8_t irq);

// Ignore (mask) a specific irq line
void pic_mask(uint8_t irq_line);

// Enable (unmask) a specific irq line
void pic_unmask(uint8_t irq_line);

