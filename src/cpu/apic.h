#pragma once
#include <stdint.h>

// Map + enable the Local APIC on the bootstrap core (does the
// one-time MMIO mapping shared by all cores).
void apic_init_bsp();

// Enable the Local APIC on an application core (mapping already done).
void apic_init_ap();

// This core's Local APIC ID (its hardware identity).
uint32_t lapic_id();

// Signal end-of-interrupt to the Local APIC (replaces the PIC's EOI
// for interrupts the LAPIC delivers, e.g. the LAPIC timer).
void lapic_eoi();

// Calibrate the LAPIC timer against the PIT (BSP, once).
void lapic_timer_calibrate();

// Arm this core's LAPIC timer to fire vector 48 periodically at the
// calibrated rate (~100 Hz). Each core calls this for its own timer.
void lapic_timer_start();

#define LAPIC_TIMER_VECTOR 48
