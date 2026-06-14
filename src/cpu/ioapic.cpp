#include "cpu/ioapic.h"
#include "cpu/apic.h"
#include "cpu/vmm.h"
#include "drivers/pic.h"
#include "utils/io.h"
#include "graphics/render.h"
#include "limine.h"
#include <stdint.h>
#include <stddef.h>

__attribute__((used, section(".limine_requests")))
volatile struct limine_rsdp_request rsdp_request = {
	.id = LIMINE_RSDP_REQUEST_ID,
	.revision = 0,
	.response = NULL
};

// Dedicated uncacheable virtual mapping for the I/O APIC MMIO page.
#define IOAPIC_VIRT 0xFFFFFFFFFEC00000

struct __attribute__((packed)) AcpiSdtHeader {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
};

struct __attribute__((packed)) Rsdp {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;
	uint32_t length;
	uint64_t xsdt_address;
};

static volatile uint32_t* ioapic = nullptr;

// ACPI table pointers are physical; reach them through the HHDM.
static uint64_t phys_to_virt(uint64_t p) {
	uint64_t off = get_hhdm_offset();
	return (p >= off) ? p : p + off;
}

static void ioapic_write(uint32_t reg, uint32_t value) {
	ioapic[0] = reg;     // IOREGSEL (offset 0x00)
	ioapic[4] = value;   // IOWIN    (offset 0x10)
}

// Route a global system interrupt to a vector on a destination LAPIC.
static void ioapic_route(uint32_t gsi, uint32_t gsi_base, uint8_t vector, uint8_t lapic_dest) {
	uint32_t entry = (gsi - gsi_base) * 2;
	// high dword: destination LAPIC id in bits 56..63 (i.e. 24..31 of high)
	ioapic_write(0x10 + entry + 1, (uint32_t)lapic_dest << 24);
	// low dword: vector, fixed delivery, physical dest, unmasked
	ioapic_write(0x10 + entry, vector);
}

void ioapic_init() {
	// Sensible defaults for a standard PC / QEMU, overridden by ACPI.
	uint64_t ioapic_phys = 0xFEC00000;
	uint32_t ioapic_gsi_base = 0;
	uint32_t kbd_gsi = 1;   // IRQ1 -> GSI1 unless an override says otherwise

	if (rsdp_request.response != NULL && rsdp_request.response->address != NULL) {
		Rsdp* rsdp = (Rsdp*)rsdp_request.response->address;

		// Walk the XSDT (ACPI 2.0+) or RSDT (1.0) for the MADT ("APIC").
		AcpiSdtHeader* madt = nullptr;
		if (rsdp->revision >= 2 && rsdp->xsdt_address) {
			AcpiSdtHeader* xsdt = (AcpiSdtHeader*)phys_to_virt(rsdp->xsdt_address);
			uint32_t n = (xsdt->length - sizeof(AcpiSdtHeader)) / 8;
			uint64_t* entries = (uint64_t*)((uint64_t)xsdt + sizeof(AcpiSdtHeader));
			for (uint32_t i = 0; i < n; i++) {
				AcpiSdtHeader* t = (AcpiSdtHeader*)phys_to_virt(entries[i]);
				if (t->signature[0]=='A' && t->signature[1]=='P' &&
				    t->signature[2]=='I' && t->signature[3]=='C') { madt = t; break; }
			}
		} else {
			AcpiSdtHeader* rsdt = (AcpiSdtHeader*)phys_to_virt(rsdp->rsdt_address);
			uint32_t n = (rsdt->length - sizeof(AcpiSdtHeader)) / 4;
			uint32_t* entries = (uint32_t*)((uint64_t)rsdt + sizeof(AcpiSdtHeader));
			for (uint32_t i = 0; i < n; i++) {
				AcpiSdtHeader* t = (AcpiSdtHeader*)phys_to_virt(entries[i]);
				if (t->signature[0]=='A' && t->signature[1]=='P' &&
				    t->signature[2]=='I' && t->signature[3]=='C') { madt = t; break; }
			}
		}

		if (madt != nullptr) {
			// Entries start after the 36-byte header + 4 (local apic addr) + 4 (flags).
			uint8_t* p = (uint8_t*)madt + sizeof(AcpiSdtHeader) + 8;
			uint8_t* end = (uint8_t*)madt + madt->length;
			while (p < end) {
				uint8_t type = p[0];
				uint8_t len = p[1];
				if (len == 0) break;
				if (type == 1) {                       // I/O APIC
					ioapic_phys = *(uint32_t*)(p + 4);
					ioapic_gsi_base = *(uint32_t*)(p + 8);
				} else if (type == 2) {                // Interrupt Source Override
					uint8_t source = p[3];
					if (source == 1) kbd_gsi = *(uint32_t*)(p + 4);
				}
				p += len;
			}
		}
	}

	// Map the I/O APIC MMIO uncacheable in the shared kernel half.
	ioapic = (volatile uint32_t*)IOAPIC_VIRT;
	kernel_vmm.map_memory((void*)IOAPIC_VIRT, (void*)ioapic_phys, PTE_CACHE_DISABLE);

	// Fully disable the legacy PIC: mask every line on both chips.
	outb(PIC1_DATA, 0xFF);
	outb(PIC2_DATA, 0xFF);

	// Route the keyboard to vector 33 on the BSP.
	ioapic_route(kbd_gsi, ioapic_gsi_base, 33, (uint8_t)lapic_id());

	terminal.printf("[OK] IOAPIC @ %x, keyboard GSI %d -> vector 33\n",
			ioapic_phys, kbd_gsi);
}
