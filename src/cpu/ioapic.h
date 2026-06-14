#pragma once

// Parse the ACPI MADT to find the I/O APIC, route the keyboard (IRQ1)
// through it to the BSP, and fully disable the legacy PIC. After this
// the keyboard's interrupt is delivered via IOAPIC -> LAPIC (EOI goes
// to the LAPIC, not the PIC).
void ioapic_init();
