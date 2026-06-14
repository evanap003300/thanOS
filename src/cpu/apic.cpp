#include "cpu/apic.h"
#include "cpu/vmm.h"
#include "utils/io.h"

#define IA32_APIC_BASE_MSR 0x1B

// Local APIC register offsets (byte offsets into its MMIO page)
#define LAPIC_REG_ID        0x20
#define LAPIC_REG_EOI       0xB0
#define LAPIC_REG_SVR       0xF0   // Spurious Interrupt Vector Register
#define LAPIC_REG_LVT_TIMER 0x320  // Local Vector Table: timer entry
#define LAPIC_REG_TIMER_INIT 0x380 // Timer initial count
#define LAPIC_REG_TIMER_CUR  0x390 // Timer current count
#define LAPIC_REG_TIMER_DIV  0x3E0 // Timer divide configuration

// How many LAPIC timer ticks elapse in 10 ms (filled by calibration).
static uint32_t lapic_ticks_per_10ms = 0;

// A dedicated, 4 KiB-mapped virtual address for the LAPIC MMIO page.
// (Don't reuse the HHDM mapping - it may be a huge page, and MMIO
// must be uncacheable.) It mirrors the usual physical base for clarity.
#define LAPIC_VIRT 0xFFFFFFFFFEE00000

static volatile uint32_t* lapic = nullptr;

static inline uint64_t rdmsr(uint32_t msr) {
	uint32_t lo, hi;
	__asm__ volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
	return ((uint64_t)hi << 32) | lo;
}

static inline void wrmsr(uint32_t msr, uint64_t value) {
	__asm__ volatile ("wrmsr" :: "c"(msr),
			  "a"((uint32_t)value), "d"((uint32_t)(value >> 32)));
}

static inline uint32_t lapic_read(uint32_t reg) {
	return lapic[reg / 4];
}

static inline void lapic_write(uint32_t reg, uint32_t value) {
	lapic[reg / 4] = value;
}

static void lapic_enable() {
	// Make sure the LAPIC is globally enabled in the MSR (bit 11)
	wrmsr(IA32_APIC_BASE_MSR, rdmsr(IA32_APIC_BASE_MSR) | (1 << 11));
	// Enable it and set the spurious vector to 0xFF (bit 8 = enable)
	lapic_write(LAPIC_REG_SVR, 0x1FF);
}

void apic_init_bsp() {
	uint64_t base_phys = rdmsr(IA32_APIC_BASE_MSR) & 0xFFFFF000;
	lapic = (volatile uint32_t*)LAPIC_VIRT;

	// Map the MMIO page uncacheable (PTE_CACHE_DISABLE). This lives
	// in the shared kernel half, so every core/process sees it.
	kernel_vmm.map_memory((void*)LAPIC_VIRT, (void*)base_phys, PTE_CACHE_DISABLE);

	lapic_enable();
}

void apic_init_ap() {
	// The mapping is shared; each core just enables its own LAPIC.
	lapic_enable();
}

uint32_t lapic_id() {
	// The terminal (which uses the reentrant lock) prints before the
	// LAPIC is mapped at boot; treat "not yet mapped" as core 0.
	if (lapic == nullptr) return 0;
	return lapic_read(LAPIC_REG_ID) >> 24;
}

void lapic_eoi() {
	lapic_write(LAPIC_REG_EOI, 0);
}

void lapic_timer_calibrate() {
	// Divide the timer input clock by 16.
	lapic_write(LAPIC_REG_TIMER_DIV, 0x3);

	// Use PIT channel 2 (the speaker timer, doesn't disturb the IRQ0
	// timer on channel 0) in one-shot mode to time a 10 ms window.
	// 1193182 Hz / 100 = ticks for 10 ms.
	uint16_t pit_count = 1193182 / 100;

	// Gate off while we program it.
	outb(0x61, inb(0x61) & 0xFE);
	// Channel 2, lo/hi byte, mode 0 (interrupt on terminal count).
	outb(0x43, 0xB0);
	outb(0x42, pit_count & 0xFF);
	outb(0x42, pit_count >> 8);

	// Start the LAPIC timer counting down from max...
	lapic_write(LAPIC_REG_TIMER_INIT, 0xFFFFFFFF);
	// ...and raise the PIT gate to start its countdown at the same time.
	outb(0x61, (inb(0x61) & 0xFE) | 0x01);

	// PIT channel 2 output (port 0x61 bit 5) goes high at terminal count.
	while (!(inb(0x61) & 0x20)) { }

	// Stop the LAPIC timer and see how far it counted in that 10 ms.
	lapic_write(LAPIC_REG_LVT_TIMER, 1 << 16);   // mask = stop
	lapic_ticks_per_10ms = 0xFFFFFFFF - lapic_read(LAPIC_REG_TIMER_CUR);
}

void lapic_timer_start() {
	lapic_write(LAPIC_REG_TIMER_DIV, 0x3);
	// Vector 48, periodic mode (bit 17).
	lapic_write(LAPIC_REG_LVT_TIMER, LAPIC_TIMER_VECTOR | (1 << 17));
	// Reload value = 10 ms worth of ticks -> ~100 Hz.
	lapic_write(LAPIC_REG_TIMER_INIT, lapic_ticks_per_10ms);
}
