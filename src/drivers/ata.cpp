#include "drivers/ata.h"
#include "utils/io.h"

// Ports for the primary ATA Bus
#define ATA_DATA        0x1F0
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LO      0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HI      0x1F5
#define ATA_DRIVE_HEAD  0x1F6
#define ATA_COMMAND     0x1F7
#define ATA_STATUS      0x1F7

// Takes in the target logical block address, amount of sectors
// on the hard disk and the buffer 
void ATA::read_sectors(uint32_t target_lba, uint8_t sector_count, uint8_t* buffer) {
	// Set Drive and top 4 bits of LBA
	outb(ATA_DRIVE_HEAD, 0xE0 | ((target_lba >> 24) & 0x0F));

	// Send NULL byte to error port
	outb(0x1F1, 0x00);

	// Send the sector count
	outb(ATA_SECTOR_CNT, sector_count); 

	// Send the low, mid, and high bytes of the LBA address
	outb(ATA_LBA_LO, (uint8_t)(target_lba & 0xFF));
	outb(ATA_LBA_MID, (uint8_t)((target_lba >> 8) & 0xFF));
	outb(ATA_LBA_HI, (uint8_t)((target_lba >> 16) & 0xFF));

	// Read sectors
	outb(ATA_COMMAND, 0x20);

	// Loop through each sector
	
	for (int s = 0; s < sector_count; s++) {
		// Make the cpu wait until the busy (0x80) bit is clear
		// and the drq (data request) bit (0x08) is set
		uint8_t status = inb(ATA_STATUS);
		while ((status & 0x80) || !(status & 0x08)) {
			status = inb(ATA_STATUS);
		}

		// Read the data (sent in 16-bit chunks)
		for (int i = 0; i < 256; i++) {
			uint16_t data = insw(ATA_DATA);
			int offset = (s * 512) + (i*2);
			buffer[offset] = (uint8_t)(data & 0xFF);
			buffer[offset + 1] = (uint8_t)((data >> 8) & 0xFF);
		}
	}
}
